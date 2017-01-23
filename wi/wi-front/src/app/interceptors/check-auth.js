(function(){
	'use strict';
	
	var app = angular.module('app');

	app.factory('checkAuthIntercept', ['$q', '$injector', function($q, $injector) {
		return {
			responseError: function(response) {
				if(response.status === 401) {
					var $state = $injector.get('$state');
					var loginState = $injector.get('loginState');
					
					if($state.current.name !== 'login') {
						loginState.nextState = $state.current.name;
                        loginState.nextStateParams = {};
						loginState.logout();
						$state.go('login');
					}
				}

				return $q.reject(response);
			}
		};
	}]);
})();