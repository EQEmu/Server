# Guide To Building From Source Without Installer

This guide is far from exhaustive, you should expect to have some experience with building C++ code before considering compiling the code from scratch.  You should instead consider using the installer scripts if you don't want to hack on the code directly.

### CMake

EQEmu uses CMake as the build system on all platforms.  You will need CMake 3.2 or higher to build from source.

### Dependencies

The following libraries are required to build from source:
- [boost](https://www.boost.org/ "boost")
- [zlib](https://www.zlib.net/ "zlib") (If not included the source will build [zlib-ng](https://github.com/zlib-ng/zlib-ng "zlib-ng") instead)
- [libmysql](https://dev.mysql.com/downloads/connector/c/ "libmysql") or [libmariadb](https://github.com/MariaDB/mariadb-connector-c "libmariadb")

The following libraries are not strictly required but in many cased recommended.
- [OpenSSL](https://www.openssl.org/ "OpenSSL") or [mbedTLS](https://tls.mbed.org/ "mbedTLS") (Required for the loginserver and headless client)
- [libsodium](https://github.com/jedisct1/libsodium "libsodium") (Required for strong password hashing on the loginserver)
- [Lua 5.1](https://www.lua.org/ "Lua 5.1") or [LuaJit](http://luajit.org/ "LuaJit") (Required for Lua Quest Scripting)
- [Perl](https://www.perl.org/ "Perl") (Required for Perl Quest Scripting)

##### Windows
For windows it is suggested you make use of [vcpkg](https://github.com/microsoft/vcpkg "vcpkg") if you wish to build your own dependencies.

If you wish to use Perl then you should use whichever version of Perl you have installed on the target system.

You can also download a vcpkg export from our releases section for Visual Studio [x86](https://github.com/EQEmu/Server/releases/download/v1.2/vcpkg-export-x86.zip "x86") or [x64](https://github.com/EQEmu/Server/releases/download/v1.2/vcpkg-export-x64.zip "x64") that includes a toolchain file you can pass to CMake.

##### Linux
For Linux you simply can install the dependencies from your package manager, below is an example of doing it on Ubuntu using apt-get.

    sudo apt-get install libmysqlclient-dev libperl-dev libboost-dev liblua5.1-0-dev zlib1g-dev uuid-dev libssl-dev

### Running CMake

##### Windows
The following is a modified command our automated build server uses to run CMake via the release vcpkg export and its toolchain file. 

Assuming it is starting in c:/projects/eqemu and the x64 dependencies were extracted to c:/projects/eqemu/vcpkg.

	mkdir build
	cd build
	cmake -G "Visual Studio 15 2017 Win64" -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_ZLIB=ON -DEQEMU_ENABLE_BOTS=ON -DCMAKE_TOOLCHAIN_FILE="c:/projects/eqemu/vcpkg/vcpkg-export-20180828-145455/scripts/buildsystems/vcpkg.cmake" ..

##### Linux
Similarly to Windows running CMake on Linux is simple it just omits the toolchain file and uses a different generator.

	mkdir build
	cd build
	cmake -G "Unix Makefiles" -DEQEMU_BUILD_TESTS=ON -DEQEMU_ENABLE_BOTS=ON -DEQEMU_BUILD_LOGIN=ON ..

### Building

##### Windows
Inside the build directory a file EQEmu.sln should be produced by a successful run of the CMake command.  You can either open this with Visual Studio or build it directly with MSBuild via the command line.

	msbuild EQEmu.sln /p:Configuration=Release

##### Linux
From the build directory you can simply call make to build.  

For example.

	make -j4
