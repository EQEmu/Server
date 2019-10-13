var RegisterAPI = function(wsi, api) {
	require('./eqw.js').Register(wsi, api);
};

module.exports = {
	'Register': RegisterAPI
}