@echo off
chcp 65001 > nul
title Scarlett Warmth - Copiar a Carpeta VST3 de FL Studio
color 0a

echo =======================================================================
echo     SCARLETT WARMTH VST3 - INSTALADOR DIRECTO EN FL STUDIO
echo =======================================================================
echo.

set "DEST_DIR=%COMMONPROGRAMFILES%\VST3"
set "PLUGIN_NAME=Scarlett Warmth Preamp.vst3"

echo Buscando archivo compilado %PLUGIN_NAME%...

set "FOUND_PATH="

if exist "build\ScarlettWarmth_artefacts\Release\VST3\%PLUGIN_NAME%" (
    set "FOUND_PATH=build\ScarlettWarmth_artefacts\Release\VST3\%PLUGIN_NAME%"
) else if exist "build\Release\%PLUGIN_NAME%" (
    set "FOUND_PATH=build\Release\%PLUGIN_NAME%"
) else if exist "build\%PLUGIN_NAME%" (
    set "FOUND_PATH=build\%PLUGIN_NAME%"
) else if exist "%PLUGIN_NAME%" (
    set "FOUND_PATH=%PLUGIN_NAME%"
)

if "%FOUND_PATH%"=="" (
    echo [ERROR] No se ha encontrado el archivo compilado %PLUGIN_NAME%.
    echo Primero debes compilar el proyecto en Visual Studio 2022 en modo Release x64.
    echo.
    echo Puedes generar y abrir la solución ejecutando: Abrir_en_VisualStudio_2022.bat
    echo.
    pause
    exit /b 1
)

echo Archivo encontrado en: %FOUND_PATH%
echo Destino oficial VST3: %DEST_DIR%
echo.

if not exist "%DEST_DIR%" (
    echo Creando carpeta VST3 en %DEST_DIR%...
    mkdir "%DEST_DIR%" 2>nul
)

echo Copiando plugin...
if exist "%FOUND_PATH%\*" (
    xcopy /E /I /Y "%FOUND_PATH%" "%DEST_DIR%\%PLUGIN_NAME%"
) else (
    copy /Y "%FOUND_PATH%" "%DEST_DIR%\"
)

if %ERRORLEVEL% equ 0 (
    echo.
    echo [ÉXITO TOTAL] ¡El plugin se ha copiado correctamente!
    echo.
    echo ===================================================================
    echo CÓMO ACTIVARLO EN FL STUDIO:
    echo 1. Abre FL Studio (20, 21 o 24).
    echo 2. Ve al menú superior: Options -^> Manage plugins.
    echo 3. Haz clic en el botón naranja "Find installed plugins".
    echo 4. Verás resaltado en la lista: "Scarlett Warmth Preamp".
    echo 5. Abre el Mixer (F9), en el canal de tu voz añade el efecto.
    echo ===================================================================
) else (
    echo.
    echo [AVISO DE PERMISOS] Windows necesita permisos de Administrador para copiar a Program Files.
    echo Por favor, haz clic derecho sobre este archivo "Instalar_en_FL_Studio.bat"
    echo y selecciona "Ejecutar como administrador".
)

echo.
pause
