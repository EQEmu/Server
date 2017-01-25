'use strict';
var $ = global.jQuery = require('jquery');
require('datatables.net')(jQuery);

var angular = require('angular');
require('angular-material');
require('angular-ui-router');
require('angular-loading-bar');
require('angular-animate');
require('ngstorage');
require('angular-datatables');

var app = angular.module('app', ['ngMaterial', 'ui.router', 'angular-loading-bar', 'ngAnimate' ,'ngStorage', 'templates', 'datatables']);

app.config(['$sceDelegateProvider', 'cfpLoadingBarProvider', '$animateProvider', '$compileProvider', '$localStorageProvider', '$sessionStorageProvider', '$httpProvider', 
    function($sceDelegateProvider, cfpLoadingBarProvider, $animateProvider, $compileProvider, $localStorageProvider, $sessionStorageProvider, $httpProvider) {
        $sceDelegateProvider.resourceUrlWhitelist([
			'self'
		]);

        $animateProvider.classNameFilter(/^((?!(fa-spinner|fa-cog|fa-refresh|fa-circle-o-notch)).)*$/);

        $compileProvider.preAssignBindingsEnabled(true);

        $localStorageProvider.setKeyPrefix('eqemu_wi_');
        $sessionStorageProvider.setKeyPrefix('eqemu_wi_');

        $httpProvider.interceptors.push('applyAuthIntercept');
        $httpProvider.interceptors.push('checkAuthIntercept');
    }]);

angular.element(function() {
    angular.bootstrap(document, ['app']);
});

require('./app/app.js');
require('./app/services');
require('./app/routes');
require('./app/components');
require('./app/interceptors');