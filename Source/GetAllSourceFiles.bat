@echo on

powershell -NoLogo -NoProfile -Command ^
    (Get-ChildItem -File -Filter * -Include *.cpp, *.h* -recurse).FullName ^| ForEach-Object { $_ -replace '\\','/'} ^| Out-File './AllSourceFiles.txt' -Encoding ascii

move AllSourceFiles.txt ../CMake/

exit