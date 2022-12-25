$cwd = Get-Location

Set-Location -Path "$cwd"

if(![System.IO.Directory]::Exists("$cwd\win-build-bots-x64")) {
	Write-Information -MessageData "Creating build (bots) x64 folder" -InformationAction Continue
	New-Item -Path "$cwd\win-build-bots-x64" -ItemType Directory
}

Write-Information -MessageData "Creating bots build x64" -InformationAction Continue
Set-Location -Path "$cwd\win-build-bots-x64"
cmake -Wno-dev -G "Visual Studio 17 2022" -A x64 -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_ZLIB=ON -DEQEMU_ENABLE_BOTS=ON "$cwd"
cmake --build . --config RelWithDebInfo --clean-first
Set-Location -Path "$cwd"
