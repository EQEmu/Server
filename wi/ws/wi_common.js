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

function RegisterSubscription(namespace, event, wsi, api) {
	wsi.Register(namespace + '::Subscribe::' + event, function(request) {
		api.Subscribe(event, request.ws);
	});
	
	wsi.Register(namespace + '::Unsubscribe::' + event, function(request) {
		api.Unsubscribe(event, request.ws);
	});
}

module.exports = {
	'Register': Register,
	'RegisterSubscription': RegisterSubscription
}