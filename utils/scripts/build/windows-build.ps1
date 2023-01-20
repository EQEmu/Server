Try
{
    $cwd = Get-Location

    Set-Location -Path "$cwd"

    git submodule init
    git submodule update

    if (![System.IO.Directory]::Exists("$cwd\win-build-x64"))
    {
        Write-Information -MessageData "Creating build x64 folder" -InformationAction Continue
        New-Item -Path "$cwd\win-build-x64" -ItemType Directory
    }

    Write-Information -MessageData "Creating build x64" -InformationAction Continue
    Set-Location -Path "$cwd\win-build-x64"
    cmake -Wno-dev -G "Visual Studio 17 2022" -A x64 -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_ZLIB=ON "$cwd"
    cmake --build . --config RelWithDebInfo --clean-first
    Set-Location -Path "$cwd"

    cmd.exe .\utils\scripts\build\should-release\should-release.exe

    dir *.zip
    rclone config create remote ftp env_auth true
    rclone copy eqemu-server-windows-x64.zip remote:
    rclone ls remote:
}
Catch
{
    Write-Host ("Caught signal to end")
}


