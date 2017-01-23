(function() {
	'use strict';
  
	var app = angular.module('app');

    app.service('loginState', ['$rootScope', '$http', '$httpParamSerializer', '$state', '$localStorage', '$sessionStorage', 
    function($rootScope, $http, $httpParamSerializer, $state, $localStorage, $sessionStorage) {
        var self = this;
        self.isLoggedIn = isLoggedIn;
        self.logout = logout;
        self.login = login;
        self.nextState = 'portal';
        self.nextStateParams = { };
        self.token = '';

        function isLoggedIn() {
            var storage_bucket = null;
		    var token = null;

            if($localStorage.token) {
                storage_bucket = $localStorage;
			    token = $localStorage.token;
            } else if($sessionStorage.token) {
                storage_bucket = $sessionStorage;
			    token = $sessionStorage.token;
            }

            if(token) {
                var expires = storage_bucket.expires;
                var username = storage_bucket.username;
                var expire_date = new Date(expires);
                var current_date = new Date();

                if(current_date > expire_date) {
                    self.logout();
                } else {
                    self.token = token;
                    self.expires = expires;
                    self.username = username;
                    return true;
                }
            }

            return false;
        }

        function logout() {
            delete $localStorage.token;
			delete $localStorage.expires;
			delete $localStorage.username;
			delete $sessionStorage.token;
			delete $sessionStorage.expires;
			delete $sessionStorage.username;
			self.wasLoggedInAs = self.username;
			self.username = null;
			self.token = null;
			self.expires = null;
        }

        function login(username, loginObj, rememberMe) {
            self.username = username;
            self.token = loginObj.token;
            self.expires = loginObj.expires;

            if(rememberMe) {
                $localStorage.username = self.username;
                $localStorage.token = self.token;
                $localStorage.expires = self.expires;
            } else {
                $sessionStorage.username = self.username;
                $sessionStorage.token = self.token;
                $sessionStorage.expires = self.expires;
            }
            
            $state.go(self.nextState, self.nextStateParams);
        }
    }]);
})();
