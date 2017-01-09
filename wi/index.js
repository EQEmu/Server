var servertalk = require('./servertalk_client.js');
var fs = require('fs');
var settings = JSON.parse(fs.readFileSync('settings.json', 'utf8'));

var client = new servertalk.client();

client.Init(settings.addr, settings.port, false, 'WebInterface', settings.key);

client.on('connecting', function(){
	console.log('Connecting...');
});

client.on('connect', function(){
	console.log('Connected...');
		
	this.Send(47, Buffer.from(JSON.stringify({ method: 'IsLocked', params: [], id: '12345' })));
	this.Send(47, Buffer.from(JSON.stringify({ method: 'Lock', params: [] })));
	this.Send(47, Buffer.from(JSON.stringify({ method: 'IsLocked', params: [], id: '12346' })));
	this.Send(47, Buffer.from(JSON.stringify({ method: 'Unlock', params: [] })));
	this.Send(47, Buffer.from(JSON.stringify({ method: 'IsLocked', params: [], id: '12347' })));
});

client.on('close', function(){
	console.log('Closed');
});

client.on('error', function(err){
	console.log(err);
});

client.on('message', function(opcode, packet) {
	console.log(Buffer.from(packet).toString('utf8'));
});
