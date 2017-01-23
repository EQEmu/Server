var endpoint = require('./endpoint.js');

var RegisterAPI = function(app, api) {
	endpoint.Register(app, api, 'item', 'items', 'id', ['name']);
};

module.exports = {
	'Register': RegisterAPI
}