const common = require('./wi_common.js');

var RegisterEQW = function(wsi, api) {
	common.Register('EQW::IsLocked', wsi, api);
	common.Register('EQW::Lock', wsi, api);
	common.Register('EQW::Unlock', wsi, api);
	common.RegisterSubscription('EQW', 'WorldZoneUpdate', wsi, api);
};

module.exports = {
	'Register': RegisterEQW
}