var RegisterAPI = function(app, api) {
	require('./eqw.js').Register(app, api);
	require('./token.js').Register(app);
	require('./data').Register(app, api);
};

module.exports = {
	'Register': RegisterAPI
}