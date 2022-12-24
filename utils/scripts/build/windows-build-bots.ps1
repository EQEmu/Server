$cwd = Get-Location

Set-Location -Path "$cwd"

if(![System.IO.Directory]::Exists("$cwd\build_bots_x64")) {
	Write-Information -MessageData "Creating build (bots) x64 folder" -InformationAction Continue
	New-Item -Path "$cwd\build_bots_x64" -ItemType Directory
}

Write-Information -MessageData "Creating bots build x64" -InformationAction Continue
Set-Location -Path "$cwd\build_bots_x64"
cmake -Wno-dev -G "Visual Studio 17 2022" -A x64 -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_ZLIB=ON -DEQEMU_ENABLE_BOTS=ON "$cwd"
cmake --build . --config RelWithDebInfo --clean-first
Set-Location -Path "$cwd"
7z a build_bots_x64.zip $cwd\build_bots_x64\bin\RelWithDebInfo\*.exe $cwd\build_bots_x64\bin\RelWithDebInfo\*.dll $cwd\build_bots_x64\bin\RelWithDebInfo\*.pdb $cwd\build_bots_x64\libs\zlibng\RelWithDebInfo\*.dll $cwd\build_bots_x64\libs\zlibng\RelWithDebInfo\*.pdb
