const WebSocket = require('ws');
const ws = new WebSocket('ws://localhost:9080');

ws.on('open', function open() {
	//ws.send(JSON.stringify({authorization: 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6IktTcHJpdGUxIiwiZXhwIjoxNDg0NzIzNDQxLCJpYXQiOjE0ODQxMTg2NDF9.Lmwm572yMWIu1DUrfer8JVvl1DGEkdnMsMFp5WDzp_A', method: 'EQW::ZoneUpdate::Subscribe'}));
	ws.send(JSON.stringify({authorization: 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6IktTcHJpdGUxIiwiZXhwIjoxNDg0NzIzNDQxLCJpYXQiOjE0ODQxMTg2NDF9.Lmwm572yMWIu1DUrfer8JVvl1DGEkdnMsMFp5WDzp_A', method: 'EQW::ClientUpdate::Subscribe'}));
});

ws.on('message', function(data, flags) {
	console.log(data);
});