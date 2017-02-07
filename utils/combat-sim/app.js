var app = angular.module('app', ['nvd3']);

app.controller('MainCtrl', function($scope, $interval) {
    $scope.options = {
        chart: {
            type: 'discreteBarChart',
            height: 450,
            margin: {
                top: 20,
                right: 20,
                bottom: 50,
                left: 55
            },
            x: function(d) {
                return d.label;
            },
            y: function(d) {
                return d.value + (1e-10);
            },
            showValues: true,
            valueFormat: function(d) {
                return d3.format(',.2r')(d);
            },
            duration: 500,
            xAxis: {
                axisLabel: 'D1-D20'
            },
            yAxis: {
                axisLabel: 'Count'
            }
        }
    };

    $scope.offense = 100;
    $scope.mitigation = 100;

    $scope.data = [{
        key: "Cumulative Return",
        values: []
    }];

    for (var i = 0; i < 20; ++i) {
        var value = {
            "label": i + 1,
            "value": 0
        };
        $scope.data[0].values.push(value);
    }

    function addRoll(interval) {
        $scope.data[0].values[interval - 1].value += 1;
    }

    var stop;
    $scope.clearData = function() {
        console.log('Clearing data');
        for (var i = 0; i < 20; ++i) {
            $scope.data[0].values[i].value = 0;
        }
    };

    $scope.start = function() {
        if (angular.isDefined(stop))
            return;

        stop = $interval(doCombatRound, 100);
    };

    $scope.stop = function() {
        if (angular.isDefined(stop)) {
            $interval.cancel(stop);
            stop = undefined;
        }
    };

    $scope.$on('$destroy', function() {
        $scope.stop();
    });

    var damage_mods = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];

    function getRandomInt(min, max) {
        min = Math.ceil(min);
        max = Math.floor(max);
        return Math.floor(Math.random() * (max - min)) + min;
    }

    function getRandom(min, max) {
        return Math.random() * (max - min) + min;
    }

    function addChance(bucket, chance, value) {
        for (var i = 0; i < chance; ++i) {
            bucket.push(value);
        }
    }

    function doCombatRound() {
        var offense = getRandomInt(0, $scope.offense + 5);
        var mitigation = getRandomInt(0, $scope.mitigation + 5);
        var avg = parseInt(($scope.offense + $scope.mitigation + 10) / 2);
        var index = parseInt((offense - mitigation) + (avg / 2));
        if (index < 0) {
            index = 0;
        }
        index = parseInt((index * 20) / avg);
        if (index >= 20)
            index = 19;
        if (index < 0)
            index = 0;
        var roll = damage_mods[index];
        addRoll(roll);
    };
});

