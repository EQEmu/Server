(function(){
	'use strict';
	
	var app = angular.module('app');

	app.factory('applyAuthIntercept', ['$injector', function($injector) {
		return {
			request: function(config) {
				if(config.anonOnly) {
				   return config; 
				}

				if(!config.headers) {
					config.headers = { };
				}

				var loginState = $injector.get('loginState');
				config.headers.Authorization = 'Bearer ' + loginState.token;

				return config;
			}
		};
	}]);
})();