$cwd = Get-Location

git submodule init
git submodule update

Set-Location -Path $cwd

if(![System.IO.Directory]::Exists("build_x64")) {
	Write-Information -MessageData "Creating build x64 folder" -InformationAction Continue
	New-Item -Path "build_x64" -ItemType Directory
}

if(![System.IO.Directory]::Exists("build_bots_x64")) {
	Write-Information -MessageData "Creating build (bots) x64 folder" -InformationAction Continue
	New-Item -Path "build_bots_x64" -ItemType Directory
}

Write-Information -MessageData "Creating no bots build x64" -InformationAction Continue
Set-Location -Path "build_x64"
cmake -Wno-dev -G "Visual Studio 17 2022" -A x64 -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_ZLIB=ON -DEQEMU_ENABLE_BOTS=OFF "\Server"
cmake --build . --config RelWithDebInfo --clean-first
Set-Location -Path "$cwd"
7z a build_x64.zip build_x64\bin\RelWithDebInfo\*.exe build_x64\bin\RelWithDebInfo\*.dll build_x64\bin\RelWithDebInfo\*.pdb build_x64\libs\zlibng\RelWithDebInfo\*.dll build_x64\libs\zlibng\RelWithDebInfo\*.pdb

Write-Information -MessageData "Creating bots build x64" -InformationAction Continue
Set-Location -Path "build_bots_x64"
cmake -Wno-dev -G "Visual Studio 17 2022" -A x64 -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_ZLIB=ON -DEQEMU_ENABLE_BOTS=ON "\Server"
cmake --build . --config RelWithDebInfo --clean-first
Set-Location -Path "$cwd"
7z a build_bots_x64.zip build_bots_x64\bin\RelWithDebInfo\*.exe build_bots_x64\bin\RelWithDebInfo\*.dll build_bots_x64\bin\RelWithDebInfo\*.pdb build_bots_x64\libs\zlibng\RelWithDebInfo\*.dll build_bots_x64\libs\zlibng\RelWithDebInfo\*.pdb
