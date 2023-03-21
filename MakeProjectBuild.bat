@echo on

mkdir Build

cd CMake

start /wait SetupCMake.bat
	     
start /wait RunCMake.bat

exit