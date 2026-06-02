@echo off
cls

if "%1"=="clean" (
	echo removing bin ...
	rmdir /s /q bin
)

:: NOTE(zgayford): for now I am building with gcc & mingw on windows
:: so that I can use pthreads
cmake -S . -B bin -G Ninja ^
	-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc ^
	-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ ^
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
if errorlevel 1 exit /b

del compile_commands.json
move bin\compile_commands.json .

cmake --build bin 
if errorlevel 1 exit /b

pushd bin
tire_auralization.exe
popd
