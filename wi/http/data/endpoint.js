var auth = require('../../core/jwt_auth.js').auth;
var sql = require('./sql.js');

var RegisterEndpoint = function(app, api, single_name, plural_name, pkey, skeys) {
	app.get('/api/data/' + single_name + '/:' + pkey, auth, function (req, res) {		
		sql.Retrieve(req, res, plural_name, pkey);
	});
	
	app.put('/api/data/' + single_name + '/:' + pkey, auth, function (req, res) {
		sql.CreateUpdate(req, res, plural_name, pkey);
	});
	
	app.delete('/api/data/' + single_name + '/:' + pkey, auth, function (req, res) {
		sql.Delete(req, res, plural_name, pkey);
	});
	
	app.post('/api/data/' + single_name + '/search', auth, function (req, res) {
		sql.Search(req, res, plural_name, pkey, skeys);
	});
};

module.exports = {
	'Register': RegisterEndpoint
}
