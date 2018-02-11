#!/bin/bash
set -e
echo "Cleaning up existing .pb* files"
rm -rf ../common/proto/* go/eqproto/* csharp/proto/*.cs java/eqproto/*.java python/proto/*pb2*
protoc --cpp_out=../common/proto --go_out=go/eqproto --python_out=python/proto --csharp_out=csharp/proto --java_out=java message.proto 