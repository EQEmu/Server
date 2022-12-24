$cwd = Get-Location

Set-Location -Path "$cwd"

7z a build_x64.zip $cwd\build_x64\bin\RelWithDebInfo\*.exe $cwd\build_x64\bin\RelWithDebInfo\*.dll $cwd\build_x64\bin\RelWithDebInfo\*.pdb $cwd\build_x64\libs\zlibng\RelWithDebInfo\*.dll $cwd\build_x64\libs\zlibng\RelWithDebInfo\*.pdb

7z a build_bots_x64.zip $cwd\build_bots_x64\bin\RelWithDebInfo\*.exe $cwd\build_bots_x64\bin\RelWithDebInfo\*.dll $cwd\build_bots_x64\bin\RelWithDebInfo\*.pdb $cwd\build_bots_x64\libs\zlibng\RelWithDebInfo\*.dll $cwd\build_bots_x64\libs\zlibng\RelWithDebInfo\*.pdb
