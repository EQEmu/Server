const fs = require('fs');
const settings = JSON.parse(fs.readFileSync('settings.json', 'utf8'));
const key = fs.readFileSync(settings.key, 'utf8');

var server;
if(settings.https.enabled) {
	const options = {
		key: fs.readFileSync(settings.https.key),
		cert: fs.readFileSync(settings.https.cert)
	};

	server = require('https').createServer();
} else {
	server = require('http').createServer();
}

const servertalk = require('./network/servertalk_api.js');
const websocket_iterface = require('./ws/ws_interface.js');
const express = require('express');
const app = express();
const bodyParser = require('body-parser');
const uuid = require('node-uuid');
const jwt = require('jsonwebtoken');
var mysql = require('mysql').createPool(settings.db);

var api = new servertalk.api();
var wsi = new websocket_iterface.wsi(server, key, api);
api.Init(settings.servertalk.addr, settings.servertalk.port, false, settings.servertalk.key);

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

app.use(function(req, res, next) {
	res.header("Access-Control-Allow-Origin", "*");
	res.header("Access-Control-Allow-Methods", "GET,HEAD,OPTIONS,POST,PUT");
	res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization");
	next();
});

//make sure all routes can see our injected dependencies
app.use(function (req, res, next) {
	req.servertalk = api;
	req.mysql = mysql;
	req.key = key;
	next();
});

app.get('/', function (req, res) {
	res.send({ status: "online" });
});

require('./http').Register(app, api);
require('./ws').Register(wsi, api);

server.on('request', app);
server.listen(settings.port, function () { console.log('Listening on ' + server.address().port) });
