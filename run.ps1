# Script de inicio para Windows (PowerShell)

Write-Host " "
Write-Host "  _  __        __          __   _____  " -ForegroundColor Cyan
Write-Host " | |/ /  ___  / _|  _   _  \ \ / / _ \ " -ForegroundColor Cyan
Write-Host " | ' /  / _ \| |_  | | | |  \ V / | | |" -ForegroundColor Cyan
Write-Host " | . \ | (_) |  _| | |_| |   | || |_| |" -ForegroundColor Cyan
Write-Host " |_|\_\ \___/|_|    \__,_|   |_| \___/ " -ForegroundColor Cyan
Write-Host "                                       "

# Cambiar al directorio del script
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location $ScriptDir

# Detectar instalación de Python
$PythonBin = Get-Command python -ErrorAction SilentlyContinue
if (-not $PythonBin) {
    Write-Host "ERROR: No se encontró Python en el sistema. Por favor, instala Python 3." -ForegroundColor Red
    exit 1
}

$VenvDir = "venv"

# 1. Crear entorno virtual si no existe
if (-not (Test-Path -Path $VenvDir)) {
    Write-Host "Creando entorno virtual en .\$VenvDir..." -ForegroundColor Yellow
    python -m venv $VenvDir
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: No se pudo crear el entorno virtual." -ForegroundColor Red
        exit 1
    }
}

# 2. Activar entorno virtual
$VenvActivate = ".\$VenvDir\Scripts\Activate.ps1"
if (Test-Path $VenvActivate) {
    & $VenvActivate
} else {
    $env:VIRTUAL_ENV = "$ScriptDir\$VenvDir"
    $env:PATH = "$ScriptDir\$VenvDir\Scripts;" + $env:PATH
}

# 3. Verificar dependencias
Write-Host "Verificando dependencias..." -ForegroundColor Yellow
python -c "import uvicorn" 2>$null
$MissingDeps = $LASTEXITCODE

if ($MissingDeps -ne 0) {
    Write-Host "Las dependencias no están instaladas en el entorno virtual." -ForegroundColor Yellow
    $Response = Read-Host "¿Deseas instalarlas ahora desde requirements.txt? (s/n)"
    if ($Response -match '^[sS]') {
        if (Test-Path "requirements.txt") {
            Write-Host "Instalando paquetes..." -ForegroundColor Green
            pip install -r requirements.txt
        } else {
            Write-Host "ERROR: No se encontró el archivo requirements.txt." -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "Cancelado. Se requiere instalar las dependencias para continuar." -ForegroundColor Red
        exit 1
    }
}

# 4. Ejecución del servicio
if (Test-Path "backend\src") {
    Set-Location "backend\src"
} else {
    Write-Host "ERROR: No se encontró el directorio backend\src" -ForegroundColor Red
    exit 1
}

Write-Host "Iniciando orquestador de Kofu en Python..." -ForegroundColor Green
python main.py