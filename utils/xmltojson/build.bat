@echo off
setlocal
set name="xmltojson"

echo Building Linux
set GOOS=linux
set GOARCH=amd64
go build -o %name%-linux-x64 main.go
set GOARCH=386
go build -o %name%-linux-x86 main.go
echo Building Windows
set GOOS=windows
set GOARCH=amd64
go build -o %name%-windows-x64.exe main.go
set GOARCH=386
go build -o %name%-windows-x86.exe main.go
echo Building OSX
REM set GOOS=darwin
REM set GOARCH=amd64
REM go build -o %name%-osx-x64 main.go
endlocal