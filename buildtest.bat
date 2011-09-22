set
call vcvars32
rmdir /s /q build
mkdir build
cd build
cmake -G "NMake Makefiles" -D CMAKE_BUILD_TYPE:STRING=Release -D ENABLE_COMPARE_RENDERER:BOOL=OFF -D VM_MACHINE_TYPE:STRING=Call ..
nmake
if errorlevel 0 (nmake test)

