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
    $scope.defense = 100;

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
        var offense = $scope.offense;
        var defense = $scope.defense;
        defense = defense - ((defense - offense) / 2.0);
        var diff = offense - defense;
        var mean = 0.0;
        var mult1 = 0.0;
        var mult2 = 0.0;

        if (offense > 30.0) {
            mult1 = offense / 200.0 + 25.75;
            if ((defense / offense) < 0.35) {
                mult1 = mult1 + 1.0;
            } else if ((defense / offense) > 0.65) {
                mult1 = mult1 - 1.0;
            }
            mult2 = offense / 140 + 18.5;
        } else {
            mult1 = 11.5 + offense / 2.0;
            mult2 = 14.0 + offense / 6.0;
        }

        if (offense > defense) {
            mean = diff / offense * mult1;
        } else if (defense > offense) {
            mean = diff / defense * mult2;
        }

        var stddev = 8.8;
        var theta = 2 * Math.PI * getRandom(0.0, 1.0);
        var rho = Math.sqrt(-2 * Math.log(1 - getRandom(0.0, 1.0)));
        var d = mean + stddev * rho * Math.cos(theta);

        if (d < -9.5) {
            d = -9.5;
        } else if (d > 9.5) {
            d = 9.5;
        }
        d = d + 11;
        addRoll(parseInt(d));
    };
});

