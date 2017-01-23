(function() {
	'use strict';
	
	var app = angular.module('app');
	
	app.component('portal', {
		templateUrl: 'app/components/portal/portal.html',
		controller: ['$state', '$mdDialog', 'loginState', function($state, $mdDialog, loginState) {
			var self = this;
			self.openMenu = openMenu;
			self.logout = logout;

			function openMenu($mdOpenMenu, ev) {
				self.originatorEv = ev;
				$mdOpenMenu(ev);
			};

			function logout(ev) {
				var confirm = $mdDialog.confirm()
					.title('Logout')
					.textContent('Are you sure you wish to logout?')
					.ariaLabel('Logout')
					.targetEvent(ev)
					.ok('Logout')
					.cancel('Cancel');

				$mdDialog.show(confirm).then(function() {
					loginState.logout();
					$state.go('login');
				}, function() {

				});
			}

            self.$onInit = function () {
            };
		}]
		});
})();
