@echo off
mkdir go\eqproto python\proto java\eqproto csharp\proto
del /q ..\common\proto\*
del /q go\eqproto\*
del /q python\proto\*
del /q java\eqproto\*
del /q csharp\proto\*
..\dependencies\protobuf\bin\protoc --go_out=go/eqproto --python_out=python/proto --csharp_out=csharp/proto --java_out=java --proto_path=../common message.proto 