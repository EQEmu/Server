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

function RegisterSubscription(event, wsi, api) {
	wsi.Register(event + '::Subscribe', function(request) {
		api.Subscribe(event, request.ws);
	});
	
	wsi.Register(event + '::Unsubscribe', function(request) {
		api.Unsubscribe(event, request.ws);
	});
}

module.exports = {
	'Register': Register,
	'RegisterSubscription': RegisterSubscription
}
