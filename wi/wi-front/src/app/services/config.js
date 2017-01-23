(function() {
	'use strict';

    var fs = require('fs'); 
	var app = angular.module('app');

    app.service('appConfig', function() {
        this.config = JSON.parse(fs.readFileSync(__dirname + '/../../../config.json', 'utf8'));
    });
})();
