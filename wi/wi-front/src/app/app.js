(function() {
	'use strict';
	
	var app = angular.module('app');
	
	app.component('appRoot', {
		templateUrl: 'app/app.html',
		controller: ['$state', function($state) {
			var self = this;

            this.$onInit = function () {
            };
		}]
		});
})();
