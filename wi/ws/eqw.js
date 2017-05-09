const common = require('./wi_common.js');

var RegisterEQW = function(wsi, api) {
	common.Register('EQW::GetConfig', wsi, api);
	common.Register('EQW::IsLocked', wsi, api);
	common.Register('EQW::Lock', wsi, api);
	common.Register('EQW::Unlock', wsi, api);
	common.Register('EQW::GetPlayerCount', wsi, api);
	common.Register('EQW::GetZoneCount', wsi, api);
	common.Register('EQW::GetLauncherCount', wsi, api);
	common.Register('EQW::GetLoginServerCount', wsi, api);
	common.RegisterSubscription('EQW::ZoneUpdate', wsi, api);
	common.RegisterSubscription('EQW::ClientUpdate', wsi, api);
};

module.exports = {
	'Register': RegisterEQW
}