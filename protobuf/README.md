# Protobuf

* [https://developers.google.com/protocol-buffers/](Learn more about protobuf)
* Version is 3.5.1 (latest). In order to modify *.pb.cpp files, you need to have the [https://github.com/google/protobuf/releases](protoc binary).
* If you add any new .proto files, you need to include them into the CMakeList.txt entry of the respective dir, likely common/CMakeList.txt.
* By default, the generated cpp files are placed in the common/proto/* directory, while this is not best practice to have them versioned as they are auto generated files, it simplifies compiling source by not requiring protoc. (Perhaps later, we can look into adding this flow into cmake)
* Run build.bat or build.sh to build protobuf for different platforms.
* Look at each language subdirectory to learn more about building them