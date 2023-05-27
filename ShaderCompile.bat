@echo off
set "GLSL_SHADER_DIR=.\assets\shaders\glsl"
set "SPV_DIR=.\assets\shaders\spv"
set "GLSLC_EXE=.\3rd\vulkanSDK\Bin\glslc.exe"

if not exist "%SPV_DIR%" mkdir "%SPV_DIR%"
for /r "%GLSL_SHADER_DIR%" %%f in (*.vert, *.frag, *.glsl) do (
    echo Compiling %%~nxf to "%SPV_DIR%\%%~nxf.glsl.spv"
    "%GLSLC_EXE%" "%%~f" -o "%SPV_DIR%\%%~nxf.glsl.spv"
)

set "HLSL_SHADER_DIR=.\assets\shaders\hlsl"
set "HLSLC_EXE=.\3rd\vulkanSDK\Bin\dxc.exe"
for /r "%HLSL_SHADER_DIR%" %%f in (*.vert) do (
    echo Compiling %%~nxf to "%SPV_DIR%\%%~nxf.hlsl.spv"
    "%HLSLC_EXE%" -E main "%%~f" -T vs_6_0 -spirv -Fo "%SPV_DIR%\%%~nxf.hlsl.spv"
)
for /r "%HLSL_SHADER_DIR%" %%f in (*.frag) do (
    echo Compiling %%~nxf to "%SPV_DIR%\%%~nxf.hlsl.spv"
    "%HLSLC_EXE%" -E main "%%~f" -T ps_6_0 -spirv -Fo "%SPV_DIR%\%%~nxf.hlsl.spv"
)

pause