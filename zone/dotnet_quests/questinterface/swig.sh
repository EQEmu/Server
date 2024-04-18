#!/bin/bash

swig -csharp -c++ -I../../../submodules/fmt/include -outfile DotNetTypes/dotnet_quests.cs questinterface.i