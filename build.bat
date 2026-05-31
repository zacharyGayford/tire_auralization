@echo off

pushd bin
clang++ ^
	../src/*.cpp ^
	-g -Wvarargs -Wextra -Wall -Werror -Wno-missing-braces ^
	-I../src ^
	-o main.exe
main.exe
popd

