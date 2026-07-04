#!/usr/bin/env bash
set -euo pipefail

# iniciar.sh - Script para iniciar Kofu en Linux (Pop!_OS / Ubuntu)
# Similar a iniciar.bat pero adaptado a entornos Linux.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "========================================"
echo "   Kofu v0.7 (Beta) - Plataforma de IA para Office"
echo "========================================"
echo
echo "Directorio del script: ${SCRIPT_DIR}"
echo

# Comprobar Ollama
if command -v ollama >/dev/null 2>&1; then
  echo "✅ Ollama encontrado!"
  echo "Iniciando Ollama en segundo plano..."
  nohup ollama serve >/dev/null 2>&1 &
  OLLAMA_PID=$!
  sleep 2
  echo "✅ Ollama listo (PID ${OLLAMA_PID})"
else
  echo "⚠️ Ollama no encontrado. Usando razonamiento básico."
  OLLAMA_PID=""
fi

echo
# Ir al directorio backend
if ! cd "${SCRIPT_DIR}/backend"; then
  echo "ERROR: No se puede acceder a ${SCRIPT_DIR}/backend"
  echo
  echo "Contenido del directorio raíz del proyecto:"
  ls -la "${SCRIPT_DIR}"
  exit 1
fi

if [ ! -f server.py ]; then
  echo "ERROR: No se encuentra server.py en: $(pwd)"
  echo
  echo "Contenido del directorio:" 
  ls -la
  echo
  exit 1
fi

echo "Iniciando servidor Kofu..."
echo
echo "Servidor en http://localhost:5000"
echo

# Abrir la interfaz web (web/index.html)
WEB_INDEX="${SCRIPT_DIR}/web/index.html"
if [ -f "${WEB_INDEX}" ]; then
  if command -v xdg-open >/dev/null 2>&1; then
    xdg-open "${WEB_INDEX}" >/dev/null 2>&1 || true
  elif command -v gio >/dev/null 2>&1; then
    gio open "${WEB_INDEX}" >/dev/null 2>&1 || true
  elif command -v gnome-open >/dev/null 2>&1; then
    gnome-open "${WEB_INDEX}" >/dev/null 2>&1 || true
  else
    echo "Abre ${WEB_INDEX} manualmente en tu navegador."
  fi
else
  echo "Aviso: no se encontró web/index.html en: ${SCRIPT_DIR}/web"
fi

# Iniciar servidor Python en segundo plano
if command -v python3 >/dev/null 2>&1; then
  nohup python3 server.py >/dev/null 2>&1 &
  PY_PID=$!
else
  if command -v python >/dev/null 2>&1; then
    nohup python server.py >/dev/null 2>&1 &
    PY_PID=$!
  else
    echo "ERROR: No se encontró python3 ni python en el sistema. Instala Python 3 e inténtalo de nuevo."
    # Detener Ollama si lo iniciamos
    if [ -n "${OLLAMA_PID}" ]; then
      kill "${OLLAMA_PID}" >/dev/null 2>&1 || true
    fi
    exit 1
  fi
fi

sleep 3

echo
if [ -n "${PY_PID:-}" ]; then
  echo "El servidor se está ejecutando en segundo plano (PID ${PY_PID})"
else
  echo "No se pudo iniciar el servidor Python. Revisa los logs en la terminal." 
fi

echo "Presiona cualquier tecla para detenerlo"
# Esperar a que el usuario presione una tecla
read -n1 -r -s

# Detener el servidor Python iniciado
if [ -n "${PY_PID:-}" ]; then
  if kill "${PY_PID}" >/dev/null 2>&1; then
    echo "Servidor detenido (PID ${PY_PID})."
  else
    echo "No se pudo detener el proceso python (PID ${PY_PID}). Puede que ya esté cerrado o que necesites permisos." 
  fi
fi

# Detener Ollama si lo iniciamos
if [ -n "${OLLAMA_PID}" ]; then
  if kill "${OLLAMA_PID}" >/dev/null 2>&1; then
    echo "Ollama detenido (PID ${OLLAMA_PID})."
  else
    echo "No se pudo detener Ollama (PID ${OLLAMA_PID}). Puede que ya esté cerrado o que necesites permisos." 
  fi
fi

echo
echo "Servidor detenido."

exit 0
