@echo off
del /q ..\common\proto\*
del /q go\eqproto\*
del /q python\proto\*pb2*
del /q java\eqproto\*.java
del /q csharp\proto\*.cs
protoc --cpp_out=../common/proto --go_out=go/eqproto --python_out=python/proto --csharp_out=csharp/proto --java_out=java message.proto 