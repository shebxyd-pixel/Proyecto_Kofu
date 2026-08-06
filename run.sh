#!/bin/bash


echo ""
echo "    _                 ___      _      "
echo "   | |    __ _ _ __  |_ _|    / \     "
echo "   | |   / _` | '__|  | |    / _ \    "
echo "   | |__| (_| | |     | |   / ___ \   "
echo "   |_____\__,_|_|    |___| /_/   \_\  "
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

echo "Verificando modelos de Ollama..."
if command -v ollama >/dev/null 2>&1; then
    models=$(ollama list | tail -n +2 | awk '{print $1}')
    if [ -z "$models" ]; then
        echo "No se encontraron modelos instalados en Ollama."
        echo "Descargando modelo por defecto (llama3:latest)..."
        ollama pull llama3:latest
    else
        echo "Modelos instalados en Ollama:"
        echo "$models"
        echo ""
        echo "¿Qué modelo deseas usar/descargar para LarIA?"
        echo "1) Descargar/Usar llama3:latest (Recomendado)"
        echo "2) Descargar/Usar gemma4:latest (Alternativa)"
        echo "3) Continuar con mis modelos instalados"
        read -p "> " choice
        if [ "$choice" == "1" ]; then
            ollama pull llama3:latest
        elif [ "$choice" == "2" ]; then
            ollama pull gemma4:latest
        fi
    fi
else
    echo "ADVERTENCIA: Ollama no está instalado. LarIA requiere Ollama para generar documentos."
fi

echo "Iniciando orquestador de LarIA en Python..."
$PYTHON_BIN main.py
