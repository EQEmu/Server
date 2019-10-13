var endpoint = require('./endpoint.js');
var auth = require('../../core/jwt_auth.js').auth;
var sql = require('./sql.js');

var RegisterAPI = function(app, api) {
	endpoint.Register(app, api, 'item', 'items', 'id');
	
	//Can register custom controller actions here.
	app.post('/api/data/item/search', auth, function (req, res) {
		sql.Search(req, res, 'items', 'id', ['id', 'name', 'icon']);
	});
};

module.exports = {
	'Register': RegisterAPI
}