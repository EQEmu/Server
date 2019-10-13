const WebSocket = require('ws');
const ws = new WebSocket('ws://localhost:9080');

ws.on('open', function open() {
	ws.send(JSON.stringify({authorization: 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6IktTcHJpdGUxIiwiZXhwIjoxNDg2MzI4MDI3LCJpYXQiOjE0ODU3MjMyMjd9.fJUeSQsxb5C13ICANox81YdE5yImkrVw-lRCP3O40-E', method: 'EQW::ZoneUpdate::Subscribe'}));
	ws.send(JSON.stringify({authorization: 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6IktTcHJpdGUxIiwiZXhwIjoxNDg2MzI4MDI3LCJpYXQiOjE0ODU3MjMyMjd9.fJUeSQsxb5C13ICANox81YdE5yImkrVw-lRCP3O40-E', method: 'EQW::ClientUpdate::Subscribe'}));
});

ws.on('message', function(data, flags) {
	console.log(data);
});