var auth = require('../core/jwt_auth.js').auth;

function RegisterFunction(path, fn, app, api) {
	app.post(path, auth, function (req, res) {
		var params = req.body.params || [];
	
		api.Call(fn, params)
			.then(function(value) {
				res.send({ response: value });
			})
			.catch(function(reason) {			
				if(reason.message) {
					res.send({ status: reason.message });
				}
				else if(reason === 'Not connected to world server.') {
					res.send({ status: 'ENCONNECTED' });
				} else {
					res.sendStatus(500);
				}
			});
	});
}

module.exports = {
	'Register': RegisterFunction
}