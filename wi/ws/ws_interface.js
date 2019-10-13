const WebSocketServer = require('ws').Server;
const jwt = require('jsonwebtoken');
const uuid = require('node-uuid');

class WebSocketInterface
{
	constructor(server, key, api) {
		this.wss = new WebSocketServer({ server: server });
		this.methods = {};
		var self = this;
		
		this.wss.on('connection', function connection(ws) {
			self.ws = ws;
			ws.uuid = uuid.v4();
			ws.on('message', function incoming(message) {
				try {
					var request = JSON.parse(message);
					request.ws = ws;
					
					if(request.method) {
						var method = self.methods[request.method];
						if(!method) {
							self.SendError(request, 'Method not found: ' + request.method);
							return;
						}
						
						if(method.requires_auth) {
							if(!request.authorization) {
								self.SendError(request, 'Authorization Required');
								return;
							}
							
							jwt.verify(request.authorization, key, function(err, decoded) {
								if(err) {
									self.SendError(request, 'Authorization Required');
									return;
								}
								
								request.token = decoded;
								method.fn(request);
							});
							
							return;
						}
						
						method.fn(request);
						
					} else {
						self.SendError(request, 'No method supplied');
					}
					
				} catch(ex) {
					console.log('Error parsing message:', ex);
					self.SendError(null, 'No method supplied');
				}
			});
			
			ws.on('close', function() {
				api.UnsubscribeAll(ws);
			});
			
			ws.on('subscriptionMessage', function(msg) {
				self.SendRaw(msg);
			});
		});
	}
	
	Register(method, fn, requires_auth) {
		var entry = { fn: fn, requires_auth: requires_auth };
		this.methods[method] = entry;
	}
	
	SendError(request, msg) {
		try {
			if(this.ws) {
				var error = {};
				
				if(request && request.id) {
					error.id = request.id;
				}
				
				error.error = msg;
				this.ws.send(JSON.stringify(error));
			}
		} catch(ex) {
			console.log(ex);
		}
	}
	
	Send(request, value) {
		try {
			if(this.ws) {
				var response = {};
				
				if(request && request.id) {
					response.id = response.id;
				}
				
				response.response = value;
				this.ws.send(JSON.stringify(response));
			}
		} catch(ex) {
			console.log(ex);
		}
	}
	
	SendRaw(obj) {
		try {
			this.ws.send(JSON.stringify(obj));
		} catch(ex) {
			console.log(ex);
		}
	}
}

module.exports = {
	'wsi': WebSocketInterface
}