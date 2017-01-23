(function() {
	'use strict';
	
	var app = angular.module('app');
	
	app.component('login', {
		templateUrl: 'app/components/login/login.html',
		controller: ['$state', 'appConfig', 'loginState', '$http', function($state, appConfig, loginState, $http) {
			var self = this;
            self.username = '';
            self.password = '';
            self.rememberMe = false;
            self.doLogin = doLogin;
            self.canLogin = canLogin;
            self.loggingIn = false;
            self.error = false;

            function doLogin() {
                if(!canLogin()) {
                    return;
                }

                self.loggingIn = true;
                self.error = false;
                var data = { username: self.username, password: self.password };
                $http.post(appConfig.config.api_url + '/api/token', data)
                    .then(function(response) {
                        self.loggingIn = false;
                        loginState.login(self.username, response.data, self.rememberMe);
                    }, function(response) {
                        self.loggingIn = false;
                        self.error = true;
                    });
            }

            function canLogin() {
                if(self.username.length === 0 || self.password.length === 0 || self.loggingIn) {
                    return false;
                }

                return true;
            }

            self.$onInit = function () {
                //If logged in just goto the next state
                if(loginState.isLoggedIn()) {
                    $state.go(loginState.nextState, loginState.nextStateParams);
                };
            };
		}]
		});
})();
