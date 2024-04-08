#!/bin/bash

SOLUTION_NAME="eq"

if [ ! -f "$SOLUTION_NAME.sln" ]; then
    echo "Solution file not found. Creating a new solution: $SOLUTION_NAME.sln"
    dotnet new sln -n "$SOLUTION_NAME"
else
    echo "Using existing solution: $SOLUTION_NAME.sln"
fi

find . -name '*.csproj' ! -path './obj/*' -exec dotnet sln "$SOLUTION_NAME.sln" add '{}' +

echo "All projects have been added to the solution."