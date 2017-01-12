const servertalk = require('./servertalk_client.js');
const uuid = require('node-uuid');

class ServertalkAPI
{
	Init(addr, port, ipv6, credentials) {
		this.client = new servertalk.client();
		this.client.Init(addr, port, ipv6, 'WebInterface', credentials);
		this.pending_calls = {};
		var self = this;
		
		this.client.on('connecting', function() {
			console.log('Connecting...');
		});
		
		this.client.on('connect', function(){
			console.log('Connected');
		});
		
		this.client.on('close', function(){
			console.log('Closed');
		});

		this.client.on('error', function(err){
		});

		this.client.on('message', function(opcode, packet) {
			var response = Buffer.from(packet).toString('utf8');
			try {
				var res = JSON.parse(response);
				
				if(res.id) {
					if(self.pending_calls.hasOwnProperty(res.id)) {
						var entry = self.pending_calls[res.id];
						
						if(res.error) {
							var reject = entry[1];
							reject(res.error);
						} else {
							var resolve = entry[0];
							resolve(res.response);
						}
						
						delete self.pending_calls[res.id];
					}
				}
				
			} catch(ex) {
				console.log('Error processing response from server:\n', ex);
			}
		});
	}
	
	Call(method, args, timeout) {
		if(!timeout) {
			timeout = 15000
		}
		
		var self = this;
		return new Promise(
			function(resolve, reject) {
				if(!self.client.Connected()) {
					reject('Not connected to world server.');
					return;
				}
				
				var id = uuid.v4();
				
				self.pending_calls[id] = [resolve, reject];
				
				var c = { id: id, method: method, params: args };
				self.client.Send(47, Buffer.from(JSON.stringify(c)));
				
				setTimeout(function() {
					delete self.pending_calls[id];
					reject('Request timed out after ' + timeout + 'ms');
				}, timeout);
			}
		);
	}
	
	Notify(method, args) {
		var c = { method: method, params: args };
		client.Send(47, Buffer.from(JSON.stringify(c)));
	}
}

module.exports = {
	'api': ServertalkAPI
}