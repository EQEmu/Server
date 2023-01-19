$cwd = Get-Location

Set-Location -Path "$cwd"

7z a eqemu-server-windows-x64.zip $cwd\win-build-x64\bin\RelWithDebInfo\*.exe $cwd\win-build-x64\bin\RelWithDebInfo\*.dll $cwd\win-build-x64\bin\RelWithDebInfo\*.pdb $cwd\win-build-x64\libs\zlibng\RelWithDebInfo\*.dll $cwd\win-build-x64\libs\zlibng\RelWithDebInfo\*.pdb
