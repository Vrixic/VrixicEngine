@echo on

cd CMake

start /wait SetupCMake.bat
	     
start /wait RunCMake.bat

timeout /T 2
move VrixicEngineBuild ../Build

exit