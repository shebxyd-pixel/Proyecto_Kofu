@echo off
setlocal enabledelayedexpansion

REM iniciar.bat - Inicia la API unificada de Kofu (api.py) en Windows

REM === REVISA QUE ESTA LÍNEA DE ABAJO ESTÉ COMPLETA Y NO TENGA ERRORES ===
SET SCRIPT_DIR=%~dp0
SET PORT=8000

echo ========================================
echo     Kofu v0.8 (Beta) - Plataforma de IA para Office
echo ========================================
echo.
echo Directorio del script: !SCRIPT_DIR!
echo.

REM Comprobar Ollama
where ollama >nul 2>&1
if exist %ERRORLEVEL% (
  echo ❌ Ollama no encontrado. La IA seguirá funcionando con razonamiento básico
  echo   o en modo 'online' si configuras OPENAI_API_KEY en backend/.env
) else (
  echo ✅ Ollama encontrado!
  echo Iniciando Ollama en segundo plano...
  start "" /b ollama serve >nul 2>&1
  REM Obtener el PID de Ollama (esto es un ejemplo simple, puede ser más complejo)
  set OLAMMA_PID=ollama_pid_value_here
  timeout /t 2 /nobreak >nul
  echo ✅ Ollama listo (PID !OLLAMA_PID!)
)

echo.
REM Cambiar al directorio backend/src
cd /d "%SCRIPT_DIR%backend\src" || (
  echo ERROR: No se puede acceder a %SCRIPT_DIR%\backend\
  cd "%SCRIPT_DIR%"
  goto :end
)

REM Activar entorno virtual si existe
if exist "venv\Scripts\activate" (
  echo 📦 Entorno virtual detectado y activado.
  call venv\Scripts\activate.bat
) else (
  echo ❌ No se encuentra el entorno virtual. Si es necesario, crea uno con `python -m venv venv`.
)

REM Comprobar la existencia de api.py
if not exist "api.py" (
  echo ERROR: No se encuentra api.py en: %cd%
  goto :end
)

SET PYTHON_BIN=""
where python3 >nul 2>&1
if !ERRORLEVEL! equ 0 SET PYTHON_BIN=python3
where python >nul 2>&1
if !ERRORLEVEL! equ 0 (
  if "!PYTHON_BIN!"=="" SET PYTHON_BIN=python
)

if "!PYTHON_BIN!"=="" (
  echo ERROR: No se encontró python3 ni python en el sistema. Instala Python 3 e inténtalo de nuevo.
  [ -n "%OLLAMA_PID%" ] && taskkill /F /PID !OLLAMA_PID!
  goto :end
)

echo Comprobando dependencias de Python...
!"%PYTHON_BIN%"! -c "import uvicorn, fastapi" >nul 2>&1 || (
  echo Instalando dependencias desde requirements.txt...
  "%PYTHON_BIN%" -m pip install -r requirements.txt
)
echo.

REM Iniciar API de Kofu (Chat)
start "" /b "%PYTHON_BIN%" -m uvicorn api:app --host 0.0.0.0 --port %PORT% >nul 2>&1
SET PY_PID=%ERRORLEVEL%

REM Iniciar Servicio de Archivos en el puerto 5000
start "" /b "%PYTHON_BIN%" -m uvicorn api:app --host 0.0.0.0 --port 5000 >nul 2>&1
SET FILES_PID=%ERRORLEVEL%

REM Abrir la interfaz web (web\index.html)
if exist "%SCRIPT_DIR%web\index.html" (
  where xdg-open >nul 2>&1
  if !ERRORLEVEL! equ 0 (
    start "" "xdg-open"
  ) else (
    echo Abre %SCRIPT_DIR%\web\index.html manualmente en tu navegador.
  )
) else (
  echo Aviso: no se encontró web\index.html en: %SCRIPT_DIR%\web
)

REM Detener servers
if "!PY_PID!" neq "" timeout /t 2 /nobreak >nul && taskkill /F /PID !PY_PID!
if "!FILES_PID!" neq "" timeout /t 2 /nobreak >nul & taskkill /F /PID !FILES_PID!

REM Detener Ollama si se inició en este script
if "!OLLAMA_PID!" neq "" (
  timeout /t 2 /nobreak >nul
  taskkill /F /PID !OLLAMA_PID!
)

:end
echo.
if exist "%QUERY_STRING%" (
  cls
) else (
  echo Servidores detenidos correctamente. Presiona cualquier tecla para salir...
  pause >nul
)
