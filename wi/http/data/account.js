var endpoint = require('./endpoint.js');
var auth = require('../../core/jwt_auth.js').auth;
var sql = require('./sql.js');

var RegisterAPI = function(app, api) {
	endpoint.Register(app, api, 'account', 'account', 'id');
	
	//Can register custom controller actions here.
	app.post('/api/data/account/search', auth, function (req, res) {
		sql.Search(req, res, 'account', 'id', ['id', 'name']);
	});
};

module.exports = {
	'Register': RegisterAPI
}