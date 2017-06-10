const jwt = require('jsonwebtoken');

var Auth = function (req, res, next) {
	var token = '';
	try {
		token = req.headers.authorization.substring(7);
	} catch(ex) {
		console.log(ex);
		res.sendStatus(401);
		return;
	}

	jwt.verify(token, req.key, function(err, decoded) {
		if(err) {
			console.log(err);
			res.sendStatus(401);
			return;
		}
		
		req.token = decoded;
		next();
	});
};

module.exports = {
	'auth': Auth
}
