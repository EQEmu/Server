var net = require('net');
var sodium = require('libsodium-wrappers');
const EventEmitter = require('events');

var ServertalkPacketType =
{
	ServertalkClientHello: 1,
	ServertalkServerHello: 2,
	ServertalkClientHandshake: 3,
	ServertalkClientDowngradeSecurityHandshake: 4,
	ServertalkMessage: 5,
};

class ServertalkClient extends EventEmitter
{
	Init(addr, port, ipv6, identifier, credentials) {
		this.m_addr = addr;
		this.m_identifier = identifier;
		this.m_credentials = credentials;
		this.m_connecting = false;
		this.m_port = port;
		this.m_ipv6 = ipv6;
		this.m_encrypted = false;
		this.m_connection = null;
		this.m_buffer = Buffer.alloc(0);
		this.m_public_key_ours = null;
		this.m_private_key_ours = null;
		this.m_nonce_ours = null;
		this.m_public_key_theirs = null;
		this.m_nonce_theirs = null;
		this.m_shared_key = null;
		
		var self = this;
		setInterval(function() { self.Connect(); }, 100);		
	}
	
	Send(opcode, p) {
		try {
			var out;
			if(this.m_encrypted) {
				if(p.length == 0) {
					p = Buffer.alloc(1);
				}
				
				out = Buffer.alloc(6);
				out.writeUInt32LE(p.length + sodium.crypto_secretbox_MACBYTES, 0);
				out.writeUInt16LE(opcode, 4);
				
				var cipher = sodium.crypto_box_easy_afternm(p, this.m_nonce_ours, this.m_shared_key);
				this.IncrementUint64(this.m_nonce_ours);
				
				out = Buffer.concat([out, Buffer.from(cipher)], out.length + cipher.length);
			} else {
				out = Buffer.alloc(6);
				out.writeUInt32LE(p.length, 0);
				out.writeUInt16LE(opcode, 4);
				out = Buffer.concat([out, p], out.length + p.length);
			}
			
			this.InternalSend(ServertalkPacketType.ServertalkMessage, out);
		} catch(ex) {
			this.emit('error', new Error(ex));
		}
	}
	
	Connected() {
		return this.m_connection && !this.m_connecting;
	}
	
	Connect() {
		if (this.m_port == 0 || this.m_connection || this.m_connecting) {
			return;
		}
		
		this.m_connecting = true;
		
		this.emit('connecting');
		
		var self = this;
		this.m_connection = net.connect({port: this.m_port, host: this.m_addr}, function() {			
			self.m_connection.on('close', function(had_error) {
				self.emit('close');
				self.m_connection = null;
				self.m_encrypted = false;
			});
			
			self.m_connection.on('data', function(buffer) {
				self.ProcessData(buffer);
			});
			
			self.SendHello();
			self.m_connecting = false;
		});
		
		this.m_connection.on('error', function() {
			self.emit('close');
			self.m_connection = null;
			self.m_connecting = false;
		});
	}
	
	ProcessData(buffer) {
		this.m_buffer = Buffer.concat([this.m_buffer, buffer], this.m_buffer.length + buffer.length);
		this.ProcessReadBuffer();
	}
	
	SendHello() {
		var p = Buffer.alloc(0);
		this.InternalSend(ServertalkPacketType.ServertalkClientHello, p);
	}
	
	InternalSend(type, p) {
		if(!this.m_connection) {
			return;
		}
		
		var out = Buffer.alloc(5);
		out.writeUInt32LE(p.length, 0);
		out.writeUInt8(type, 4);
		
		if (p.length > 0) {
			out = Buffer.concat([out, p], out.length + p.length);
		}
		
		this.m_connection.write(out);
	}
	
	ProcessReadBuffer() {
		var current = 0;
		var total = this.m_buffer.length;
		
		while (current < total) {
			var left = total - current;
			
			var length = 0;
			var type = 0;
			if (left < 5) {
				break;
			}
			
			length = this.m_buffer.readUInt32LE(current);
			type = this.m_buffer.readUInt8(current + 4);
			
			if (current + 5 + length > total) {
				break;
			}
			
			if (length == 0) {
				var p = Buffer.alloc(0);
				switch (type) {
				case ServertalkPacketType.ServertalkServerHello:
					this.ProcessHello(p);
					break;
				case ServertalkPacketType.ServertalkMessage:
					this.ProcessMessage(p);
					break;
				}
			}
			else {
				var p = this.m_buffer.slice(current + 5, current + 5 + length);
				switch (type) {
				case ServertalkPacketType.ServertalkServerHello:
					this.ProcessHello(p);
					break;
				case ServertalkPacketType.ServertalkMessage:
					this.ProcessMessage(p);
					break;
				}
			}
	
			current += length + 5;
		}
		
		if (current == total) {
			this.m_buffer = Buffer.alloc(0);
		}
		else {
			this.m_buffer = this.m_buffer.slice(current);
		}
	}
	
	ProcessHello(p) {
		this.m_encrypted = false;
		this.m_public_key_ours = null;
		this.m_public_key_theirs = null;
		this.m_private_key_ours = null;
		this.m_nonce_ours = null;
		this.m_nonce_theirs = null;
		this.m_shared_key = null;
		
		try {
			var enc = p.readUInt8(0) == 1 ? true : false;
			if (enc) {
				if (p.length == (1 + sodium.crypto_box_PUBLICKEYBYTES + sodium.crypto_box_NONCEBYTES)) {
					this.m_public_key_theirs = p.slice(1, 1 + sodium.crypto_box_PUBLICKEYBYTES);
					this.m_nonce_theirs = p.slice(1 + sodium.crypto_box_PUBLICKEYBYTES, 1 + sodium.crypto_box_PUBLICKEYBYTES + sodium.crypto_box_NONCEBYTES);
					this.m_encrypted = true;
					this.SendHandshake(false);
					
					this.emit('connect');
				}
				else {
					this.emit('error', new Error('Could not process hello, size !=', 1 + sodium.crypto_box_PUBLICKEYBYTES + sodium.crypto_box_NONCEBYTES));
				}
			} else {
				this.SendHandshake(false);
				
				this.emit('connect');
			}
		} catch(ex) {
			this.emit('error', new Error(ex));
		}
	}
	
	ProcessMessage(p) {
		try {
			var length = p.readUInt32LE(0);
			var opcode = p.readUInt16LE(4);
			if(length > 0) {
				var data = p.slice(6);
				
				if(this.m_encrypted) {
					var message_len = length - sodium.crypto_secretbox_MACBYTES;

					var decrypted = sodium.crypto_box_open_easy_afternm(data, this.m_nonce_theirs, this.m_shared_key);

					this.IncrementUint64(this.m_nonce_theirs);
					
					this.emit('message', opcode, decrypted);
				} else {
					this.emit('message', opcode, data);
				}
			} else {
				this.emit('message', opcode, Buffer.alloc(0));
			}
		} catch(ex) {
			this.emit('error', new Error(ex));
		}
	}
	
	SendHandshake() {
		var handshake;
		
		if(this.m_encrypted) {
			var keypair = sodium.crypto_box_keypair();	
			this.m_public_key_ours = keypair.publicKey;
			this.m_private_key_ours = keypair.privateKey;
			this.m_nonce_ours = Buffer.from(sodium.randombytes_buf(sodium.crypto_box_NONCEBYTES));
			this.m_shared_key = sodium.crypto_box_beforenm(this.m_public_key_theirs, this.m_private_key_ours);
			
			this.m_public_key_theirs = null;
			this.m_private_key_ours = null;
					
			var message = Buffer.alloc(this.m_identifier.length + this.m_credentials.length + 2);
			message.write(this.m_identifier, 0);
			message.write(this.m_credentials, this.m_identifier.length + 1);
			
			var ciphertext = sodium.crypto_box_easy_afternm(message, this.m_nonce_ours, this.m_shared_key);
			
			handshake = Buffer.concat([Buffer.from(this.m_public_key_ours), Buffer.from(this.m_nonce_ours), Buffer.from(ciphertext)], sodium.crypto_box_PUBLICKEYBYTES + sodium.crypto_box_NONCEBYTES + ciphertext.length);		
			this.IncrementUint64(this.m_nonce_ours);
			
			this.m_public_key_ours = null;
		} else {
			handshake = Buffer.alloc(this.m_identifier.length + this.m_credentials.length + 2);
			handshake.write(this.m_identifier, 0);
			handshake.write(this.m_credentials, this.m_identifier.length() + 1);
		}

		this.InternalSend(ServertalkPacketType.ServertalkClientHandshake, handshake);
	}
	
	IncrementUint64(value) {	
		var bytes = [];
		for(var i = 0; i < 8; ++i) {
			bytes[i] = value[i];
		}
				
		bytes[0] += 1;
		for(i = 0; i < 7; ++i) {
			if(bytes[i] >= 0x100) {
				bytes[0] = 0;
				bytes[i + 1] += 1;
			}
		}
		
		if(bytes[7] >= 0x100) {
			bytes[7] = 0;
		}
		
		for(var i = 0; i < 8; ++i) {
			value[i] = bytes[i];
		}
	}
}

module.exports = {
	'client': ServertalkClient
}