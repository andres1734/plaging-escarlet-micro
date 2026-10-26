@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
title Scarlett Warmth - Visual Studio 2022
color 0b

echo =======================================================================
echo     SCARLETT WARMTH VST3 - PROYECTO PARA VISUAL STUDIO
echo =======================================================================
echo.

:: 1. COMPROBAR SI EL ARCHIVO FUE ABIERTO DENTRO DEL .ZIP SIN EXTRAER
if not exist "CMakeLists.txt" (
    echo [ERROR CRITICO: ARCHIVO NO DESCOMPRIMIDO]
    echo -------------------------------------------------------------------
    echo Has abierto este archivo .bat directamente dentro del archivo ZIP.
    echo Windows bloquea la ejecucion en carpetas temporales de ZIP.
    echo.
    echo SOLUCION EN 2 PASOS SENCILLOS:
    echo 1. Haz clic DERECHO en el archivo .zip que descargaste.
    echo 2. Selecciona "Extraer todo..." (Extract All) y elige una carpeta.
    echo 3. Entra en esa carpeta y vuelve a abrir este archivo.
    echo -------------------------------------------------------------------
    echo.
    pause
    exit /b 1
)

echo [OK] Carpeta descomprimida correctamente.
echo.

:: 2. VERIFICAR CMAKE (SISTEMA O INTEGRADO EN VISUAL STUDIO)
set "CMAKE_BIN="

where cmake >nul 2>nul
if %ERRORLEVEL% equ 0 (
    set "CMAKE_BIN=cmake"
    echo [1/3] CMake detectado en el sistema: OK
) else (
    echo [1/3] Buscando CMake integrado en tu instalacion de Visual Studio...
    
    for %%P in (
        "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "%ProgramFiles%\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "%ProgramFiles%\Microsoft Visual Studio\17\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    ) do (
        if exist %%P (
            set "CMAKE_BIN=%%P"
            echo [OK] CMake de Visual Studio encontrado en: %%P
            goto :found_cmake
        )
    )
)

:found_cmake
if "%CMAKE_BIN%"=="" (
    echo.
    echo ===================================================================
    echo  AVISO: CMAKE NO ENCONTRADO EN PATH
    echo ===================================================================
    echo  No te preocupes, puedes abrir el proyecto directamente en
    echo  Visual Studio (el moradito) SIN usar la consola:
    echo.
    echo  METODO DIRECTO EN VISUAL STUDIO:
    echo  1. Abre Visual Studio desde tu menu Inicio.
    echo  2. Haz clic en: "Abrir una carpeta local" (Open a local folder).
    echo  3. Elige esta carpeta: "%~dp0"
    echo  4. Listo! Visual Studio detecta CMakeLists.txt y compila a 64-bit.
    echo.
    echo  O TAMBIEN:
    echo  Haz doble clic en el archivo "ScarlettWarmth.sln" que viene incluido.
    echo ===================================================================
    echo.
    
    echo Intentando abrir Visual Studio ahora mismo...
    start devenv "%~dp0" 2>nul
    if exist "ScarlettWarmth.sln" (
        start "" "ScarlettWarmth.sln"
    )
    echo.
    pause
    exit /b 0
)

echo.
echo [2/3] Generando solucion de 64 bits para Visual Studio...
if not exist build mkdir build
cd build

"%CMAKE_BIN%" .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% neq 0 (
    echo.
    echo [AVISO] CMake reporto un mensaje. Intentando abrir solucion existente o carpeta...
    if exist "ScarlettWarmth.sln" (
        start "" "ScarlettWarmth.sln"
    ) else (
        start devenv .. 2>nul
    )
    echo.
    pause
    exit /b 0
)

echo.
echo [3/3] Solucion generada con exito!
echo Abriendo en Visual Studio 2022...

for %%F in (*.sln) do (
    echo Abriendo: %%F
    start "" "%%F"
)

echo.
echo =======================================================================
echo PASOS SIGUIENTES DENTRO DE VISUAL STUDIO:
echo 1. Arriba en la barra selecciona: "Release" y "x64"
echo 2. Presiona Ctrl + Shift + B (Compilar solucion)
echo 3. El archivo .vst3 se creara y se copiara a:
echo    C:\Program Files\Common Files\VST3\Scarlett Warmth Preamp.vst3
echo =======================================================================
echo.
pause
