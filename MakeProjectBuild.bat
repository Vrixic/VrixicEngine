@echo on

cd CMake

start /wait SetupCMake.bat
	     
start /wait RunCMake.bat

cd ../
mkdir Build
cd CMake

timeout /T 2
move VrixicEngineBuild ../Build

exit