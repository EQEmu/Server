const common = require('./common.js');

var RegisterEQW = function(app, api) {
	common.Register('/api/eqw/getconfig', 'EQW::GetConfig', app, api);
	common.Register('/api/eqw/islocked', 'EQW::IsLocked', app, api);
	common.Register('/api/eqw/lock', 'EQW::Lock', app, api);
	common.Register('/api/eqw/unlock', 'EQW::Unlock', app, api);
	common.Register('/api/eqw/getplayercount', 'EQW::GetPlayerCount', app, api);
	common.Register('/api/eqw/getzonecount', 'EQW::GetZoneCount', app, api);
	common.Register('/api/eqw/getlaunchercount', 'EQW::GetLauncherCount', app, api);
	common.Register('/api/eqw/getloginservercount', 'EQW::GetLoginServerCount', app, api);
};

module.exports = {
	'Register': RegisterEQW
}
