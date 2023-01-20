.\utils\scripts\build\should-release\should-release.exe || echo Not releasing, exit gracefully && exit /b 0

git submodule init
git submodule update
utils/scripts/build/windows-build.ps1
utils/scripts/build/package-artifacts.ps1
dir *.zip
rclone config create remote ftp env_auth true
rclone copy eqemu-server-windows-x64.zip remote:
rclone ls remote:
