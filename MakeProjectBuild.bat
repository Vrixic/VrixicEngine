@echo on

cd CMake

start /wait SetupCMake.bat
	     
start /wait RunCMake.bat

move VrixicEngineBuild ../

exit