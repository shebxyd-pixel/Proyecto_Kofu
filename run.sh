#!/bin/bash

echo "=========================================="
echo "      Iniciando Kofu para Linux..."
echo "=========================================="                                               


SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

if command -v python3 >/dev/null 2>&1; then
    PYTHON_BIN="python3"
    PIP_BIN="pip3"
elif command -v python >/dev/null 2>&1; then
    PYTHON_BIN="python"
    PIP_BIN="pip"
else
    echo "ERROR: No se encontró Python. Por favor, instala Python 3."
    exit 1
fi

VENV_DIR="env"

echo "Verificando entorno virtual..."
if [ ! -d "$VENV_DIR" ]; then
    echo "Intentando crear entorno virtual en ./$VENV_DIR..."
    if ! $PYTHON_BIN -m venv $VENV_DIR; then
        echo "ERROR: Falló la creación del entorno virtual."
        echo "Si estás en Ubuntu/Debian, puede que necesites instalar el paquete de venv ejecutando:"
        echo "sudo apt update && sudo apt install python3-venv"
        rm -rf "$VENV_DIR"
        exit 1
    fi
fi

if [ -f "$VENV_DIR/bin/activate" ]; then
    source "$VENV_DIR/bin/activate"
    PYTHON_BIN="python"
    PIP_BIN="pip"
else
    echo "ERROR: No se encontró el script de activación del entorno virtual."
    exit 1
fi

echo "Verificando dependencias..."
$PYTHON_BIN -c "import uvicorn" >/dev/null 2>&1 || {
    echo "Instalando dependencias necesarias (FastAPI, Uvicorn, etc)..."
    if ! $PIP_BIN install -r requirements.txt; then
        echo "ERROR: Falló la instalación de dependencias."
        exit 1
    fi
}

cd backend/src

echo "Iniciando orquestador de Kofu en Python..."
$PYTHON_BIN main.py
