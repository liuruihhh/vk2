@echo off
set "SHADER_DIR=.\assets\shaders"
set "SPV_DIR=%SHADER_DIR%\spv"
set "GLSLC_EXE=.\3rd\vulkanSDK\Bin\glslc.exe"

if not exist "%SPV_DIR%" mkdir "%SPV_DIR%"
for /r "%SHADER_DIR%" %%f in (*.vert, *.frag, *.glsl) do (
    echo Compiling %%~nxf to "%SPV_DIR%\%%~nxf.spv"
    "%GLSLC_EXE%" "%%~f" -o "%SPV_DIR%\%%~nxf.spv"
)

pause