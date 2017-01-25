var auth = require('../../core/jwt_auth.js').auth;
var sql = require('./sql.js');

var RegisterEndpoint = function(app, api, endpoint_verb, table_name, pkey) {
	app.get('/api/data/' + endpoint_verb + '/:' + pkey, auth, function (req, res) {		
		sql.Retrieve(req, res, table_name, pkey);
	});
	
	app.put('/api/data/' + endpoint_verb + '/:' + pkey, auth, function (req, res) {
		sql.CreateUpdate(req, res, table_name, pkey);
	});
	
	app.delete('/api/data/' + endpoint_verb + '/:' + pkey, auth, function (req, res) {
		sql.Delete(req, res, table_name, pkey);
	});
};

module.exports = {
	'Register': RegisterEndpoint
}
