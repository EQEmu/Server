const servertalk = require('./servertalk_client.js');
const uuid = require('node-uuid');

class ServertalkAPI
{
	Init(addr, port, ipv6, credentials) {
		this.client = new servertalk.client();
		this.client.Init(addr, port, ipv6, 'WebInterface', credentials);
		this.pending_calls = {};
		this.subscriptions = {};
		var self = this;
		
		this.client.on('connecting', function() {
			//console.log('Connecting...');
		});
		
		this.client.on('connect', function(){
			//console.log('Connected');
		});
		
		this.client.on('close', function(){
			//console.log('Closed');
		});

		this.client.on('error', function(err){
		});

		this.client.on('message', function(opcode, packet) {
			if(opcode == 47) {
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
			} else if(opcode == 104) {
				var message = Buffer.from(packet).toString('utf8');
				try {
					var msg = JSON.parse(message);
					
					if(msg.event) {
						if(self.subscriptions.hasOwnProperty(msg.event)) {
							var subs = self.subscriptions[msg.event];
						
							for(var idx in subs) {
								try {
									var sub = subs[idx];
									sub.emit('subscriptionMessage', msg);
								} catch(ex) {
									console.log('Error dispatching subscription message', ex);
								}
							}
						}
					}
				} catch(ex) {
					console.log('Error processing response from server:\n', ex);
				}
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
	
	Subscribe(event_id, who) {
		this.Unsubscribe(event_id, who);
		
		var subs = this.subscriptions[event_id];
		if(subs) {
			//console.log('Subscribe', who.uuid, 'to', event_id);
			subs[who.uuid] = who;
		} else {
			//console.log('Subscribe', who.uuid, 'to', event_id);
			this.subscriptions[event_id] = { };
			this.subscriptions[event_id][who.uuid] = who;
			//Tell our server we have a subscription for event_id
		}
	}
	
	Unsubscribe(event_id, who) {
		var subs = this.subscriptions[event_id];
		if(subs) {
			//console.log('Unsubscribe', who.uuid, 'from', event_id);
			delete subs[who.uuid];
			
			if(Object.keys(subs).length === 0) {
				delete this.subscriptions[event_id];
				//Tell our server we no longer have a subscription for event_id
			}
		}
	}
	
	UnsubscribeAll(who) {
		for(var sub_idx in this.subscriptions) {
			this.Unsubscribe(sub_idx, who);
		}
	}
}

module.exports = {
	'api': ServertalkAPI
}