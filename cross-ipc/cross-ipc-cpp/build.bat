@echo off
echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 (
    echo Failed to set up Visual Studio environment.
    echo Please adjust the path to vcvarsall.bat in this script to match your Visual Studio installation.
    exit /b 1
)

echo Creating output directory...
if not exist bin mkdir bin

echo Compiling library source files...
for %%f in (src\*.cpp) do (
    echo Compiling %%f...
    cl.exe /c /EHsc /std:c++14 /Iinclude /Fo"bin\%%~nf.obj" %%f
    if errorlevel 1 goto error
)

echo Compiling counter_incrementer example...
cl.exe /c /EHsc /std:c++14 /Iinclude /Fo"bin\counter_incrementer.obj" examples\counter_incrementer.cpp
if errorlevel 1 goto error

echo Creating counter_incrementer executable...
cl.exe /EHsc /std:c++14 /Febin\counter_incrementer.exe bin\counter_incrementer.obj bin\cross_ipc.obj bin\named_pipe.obj bin\shared_memory.obj bin\shm_dispenser_pattern.obj bin\store_dict_pattern.obj
if errorlevel 1 goto error

echo Copying DLL to output directory...
if exist lib\cross-ipc.dll (
    copy lib\cross-ipc.dll bin\
) else (
    echo Warning: cross-ipc.dll not found in lib directory.
    echo Please ensure the DLL is available at runtime.
)

echo Build completed successfully!
echo Run the example with: bin\counter_incrementer.exe
goto end

:error
echo Build failed!
exit /b 1

:end 