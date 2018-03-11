Hello World NATS example

## Go helloworld.go
* Install go and set your GOPATH/GOROOT variables as instructions notes. https://golang.org/doc/install
* run `go get ./...` in this directory
* run `go run helloworld.go` in this directory

## Python 2 helloworld2.py
* 2.7+ Python ideal
* run `pip install nats-client`
* run `pip install protobuf`

## C#
* The easiest way how to use C# protobuf is via the Google.Protobuf NuGet package. Just add the NuGet package to your VS project.
* Copy Message.cs to your project.
* NATS is obtained via the NATS.Client NuGet project
* (Optional) You will also want to install the Google.Protobuf.Tools NuGet package, which contains precompiled version of protoc.exe and a copy of well known .proto files under the package's tools directory.
* (Optional) To generate C# files from your .proto files, invoke protoc with the --csharp_out option.
* Read https://github.com/google/protobuf/tree/master/csharp for more details
