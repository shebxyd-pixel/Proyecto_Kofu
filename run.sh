#!/bin/bash

echo ""
echo " _  __           __                      ___         ___  "
echo "| |/ /   ___    / _|  _   _    __   __  / _ \       ( _ ) "
echo "| ' /   / _ \  | |_  | | | |   \ \ / / | | | |      / _ \ "
echo "| . \  | (_) | |  _| | |_| |    \ V /  | |_| |  _  | (_) |"
echo "|_|\_\  \___/  |_|    \__,_|     \_/    \___/  (_)  \___/ "
echo "                                                          "
echo ""
                                                           


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

echo "Verificando dependencias..."
$PYTHON_BIN -c "import uvicorn" >/dev/null 2>&1 || {
    echo "Instalando dependencias necesarias (FastAPI, Uvicorn, etc)..."
    $PIP_BIN install -r requirements.txt --user || $PIP_BIN install -r requirements.txt --break-system-packages
}

cd backend/src

echo "Iniciando orquestador de Kofu en Python..."
$PYTHON_BIN main.py
