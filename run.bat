@echo off
setlocal enabledelayedexpansion

echo ""
echo "  _              ___    _     "  
echo " | |    __ _ _ _|_ _|  / \    "  
echo " | |   / _` | '__| |  / _ \   "  
echo " | |__| (_| | |  | | / ___ \  " 
echo " |_____\__,_|_| |___/_/   \_\ "
echo ""                            

cd /d "%~dp0"

where python >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: No se encontro Python en el sistema. Por favor, instala Python 3.
    pause
    exit /b 1
)

set "VENV_DIR=venv"

rem 1. Crear entorno virtual si no existe
if not exist "%VENV_DIR%" (
    echo Creando entorno virtual en .\%VENV_DIR%...
    python -m venv %VENV_DIR%
    if %errorlevel% neq 0 (
        echo ERROR: No se pudo crear el entorno virtual.
        pause
        exit /b 1
    )
)

rem 2. Activar entorno virtual
call .\%VENV_DIR%\Scripts\activate.bat

rem 3. Verificar dependencias
echo Verificando dependencias...
python -c "import uvicorn" >nul 2>&1
if %errorlevel% neq 0 (
    echo Las dependencias no estan instaladas en el entorno virtual.
    set /p "RESP=¿Deseas instalarlas ahora desde requirements.txt? (s/n): "
    if /i "!RESP!"=="s" (
        if exist "requirements.txt" (
            echo Instalando paquetes...
            pip install -r requirements.txt
        ) else (
            echo ERROR: No se encontro el archivo requirements.txt.
            pause
            exit /b 1
        )
    ) else (
        echo Cancelado. Se requiere instalar las dependencias para continuar.
        pause
        exit /b 1
    )
)

rem 4. Ejecución del servicio
if exist "backend\src" (
    cd backend\src
) else (
    echo ERROR: No se encontro el directorio backend\src
    pause
    exit /b 1
)

echo Verificando modelos de Ollama...
where ollama >nul 2>nul
if %errorlevel% neq 0 (
    echo ADVERTENCIA: Ollama no esta instalado en el sistema. LarIA requiere Ollama para funcionar.
) else (
    for /f "skip=1" %%i in ('ollama list') do set HAS_MODELS=1
    if not defined HAS_MODELS (
        echo No se encontraron modelos en Ollama. Descargando llama3:latest por defecto...
        ollama pull llama3:latest
    ) else (
        echo Modelos instalados:
        ollama list
        echo.
        echo Opciones de Modelo:
        echo 1. Descargar/Usar llama3:latest (Recomendado)
        echo 2. Descargar/Usar gemma4:latest (Alternativa)
        echo 3. Continuar con mis modelos actuales
        set /p MCHOICE="Elige una opcion (1-3): "
        if "!MCHOICE!"=="1" ollama pull llama3:latest
        if "!MCHOICE!"=="2" ollama pull gemma4:latest
    )
)

echo Iniciando orquestador de LarIA en Python...
python main.py
pause