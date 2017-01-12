function Register(name, wsi, api) {
	wsi.Register(name, 
		function(request) {
			api.Call(name, request.params)
				.then(function(value) {
					wsi.Send(request, value);
				})
				.catch(function(reason) {
					wsi.SendError(request, reason);
				});
		}, true);
}

var RegisterEQW = function(wsi, api) {
	Register('EQW::IsLocked', wsi, api);
	Register('EQW::Lock', wsi, api);
	Register('EQW::Unlock', wsi, api);
};

module.exports = {
	'Register': RegisterEQW
}