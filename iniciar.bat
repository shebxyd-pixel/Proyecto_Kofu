@echo off
setlocal enabledelayedexpansion

echo ========================================
echo    Kofu v0.7 (Beta) - Plataforma de IA para Office
echo ========================================
echo.
echo Directorio actual: %~dp0
echo.

cd /d "%~dp0backend"

if not exist "server.py" (
    echo ERROR: No se encuentra server.py en: %CD%
    echo.
    echo Contenido del directorio:
    dir
    echo.
    pause
    exit /b 1
)

echo Iniciando servidor...
echo.
echo Iniciando servidor en http://localhost:5000
echo Esperando a que el servidor este listo...
echo.

py server.py

timeout /t 3 /nobreak >nul

echo Abriendo web/index.html...
start "" "%~dp0web\index.html"

echo.
echo El servidor se esta ejecutando en segundo plano
echo Presiona cualquier tecla para detenerlo
pause >nul

taskkill /F /IM python.exe 2>nul

echo.
echo Servidor detenido.
echo.

endlocal
