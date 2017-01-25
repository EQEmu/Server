var endpoint = require('./endpoint.js');

var RegisterAPI = function(app, api) {
	endpoint.Register(app, api, 'account', 'account', 'id');
	
	//Can register custom controller actions here.
};

module.exports = {
	'Register': RegisterAPI
}