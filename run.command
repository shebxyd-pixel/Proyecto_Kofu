#!/bin/bash
# Script de inicio para macOS

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "      Iniciando Kofu para macOS..."
echo "=========================================="

chmod +x run.sh

./run.sh

if [ $? -ne 0 ]; then
    echo ""
    echo "Ocurrió un error al iniciar Kofu."
    read -p "Presiona Enter para salir..."
fi
