const sodium = require('libsodium-wrappers-sumo');
const jwt = require('jsonwebtoken');

var RegisterToken = function(app) {
	app.post('/api/token', function (req, res) {
		try {
			req.mysql.getConnection(function(err, connection) {
				if(err) {
					console.log(err);
					res.sendStatus(500);
					connection.release();
					return;
				}
				
				connection.query('SELECT password FROM account WHERE name = ? LIMIT 1', [req.body.username], function (error, results, fields) {
					if(results.length == 0) {
						res.sendStatus(401);
						connection.release();
						return;
					}
					
					
					if(sodium.crypto_pwhash_str_verify(results[0].password, req.body.password)) {
						var expires = Math.floor(Date.now() / 1000) + (60 * 60 * 24 * 7);
						var token = jwt.sign({ username: req.body.username, exp: expires }, req.key);
						res.send({token: token, expires: expires});
						connection.release();
					} else {
						res.sendStatus(401);
						connection.release();
					}
				});
			});
		} catch(ex) {
			res.sendStatus(500);
			console.log(ex);
		}
	});
};

module.exports = {
	'Register': RegisterToken
}