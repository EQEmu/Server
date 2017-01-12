var auth = require('../core/jwt_auth.js').auth;

var RegisterEQW = function(app, api) {
	app.post('/api/eqw/islocked', auth, function (req, res) {
		api.Call('EQW::IsLocked', [])
			.then(function(value) {
				res.send({ response: value });
			})
			.catch(function(reason) {
				res.sendStatus(500);
			});
	});
	
	app.post('/api/eqw/lock', auth, function (req, res) {
		api.Call('EQW::Lock', [])
			.then(function(value) {
				res.send({ response: value });
			})
			.catch(function(reason) {
				res.sendStatus(500);
			});
	});
	
	app.post('/api/eqw/unlock', auth, function (req, res) {
		api.Call('EQW::Unlock', [])
			.then(function(value) {
				res.send({ response: value });
			})
			.catch(function(reason) {
				res.sendStatus(500);
			});
	});
};

module.exports = {
	'Register': RegisterEQW
}