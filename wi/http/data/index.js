var RegisterAPI = function(app, api) {
	require('./account.js').Register(app, api);
	require('./item.js').Register(app, api);
};

module.exports = {
	'Register': RegisterAPI
}