# Script de inicio para Windows (PowerShell)

Write-Host " "
Write-Host "  _              ___    _     " -ForegroundColor Cyan
Write-Host " | |    __ _ _ _|_ _|  / \    " -ForegroundColor Cyan
Write-Host " | |   / _` | '__| |  / _ \   " -ForegroundColor Cyan
Write-Host " | |__| (_| | |  | | / ___ \  " -ForegroundColor Cyan
Write-Host " |_____\__,_|_| |___/_/   \_\ " -ForegroundColor Cyan
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

Write-Host "Verificando modelos de Ollama..." -ForegroundColor Yellow
$OllamaBin = Get-Command ollama -ErrorAction SilentlyContinue
if (-not $OllamaBin) {
    Write-Host "ADVERTENCIA: Ollama no está instalado en el sistema. LarIA requiere Ollama para funcionar." -ForegroundColor Red
} else {
    $OllamaModels = (ollama list | Select-Object -Skip 1)
    if (-not $OllamaModels) {
        Write-Host "No se encontraron modelos en Ollama. Descargando llama3:latest por defecto..." -ForegroundColor Yellow
        ollama pull llama3:latest
    } else {
        Write-Host "Modelos instalados en Ollama:" -ForegroundColor Green
        ollama list
        Write-Host ""
        Write-Host "Opciones de Modelo:" -ForegroundColor Cyan
        Write-Host "1. Descargar/Usar llama3:latest (Recomendado)"
        Write-Host "2. Descargar/Usar gemma4:latest (Alternativa)"
        Write-Host "3. Continuar con mis modelos actuales"
        $Response = Read-Host "Elige una opción (1-3)"
        if ($Response -eq "1") { ollama pull llama3:latest }
        elseif ($Response -eq "2") { ollama pull gemma4:latest }
    }
}

Write-Host "Iniciando orquestador de LarIA en Python..." -ForegroundColor Green
python main.py