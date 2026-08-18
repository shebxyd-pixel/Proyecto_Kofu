import os
import socket

DEFAULT_OLLAMA_URL = os.getenv("OLLAMA_BASE_URL", "http://localhost:11434")

DEFAULT_OLLAMA_MODELS = [
    os.getenv("OLLAMA_MODEL_PRIMARY", "gemma4:latest"),
    os.getenv("OLLAMA_MODEL_FALLBACK", "llama3:latest"),
]

OLLAMA_TIMEOUT_CHECK = 5
OLLAMA_TIMEOUT_GENERATE = 120

NO_MODEL_ERROR_CODE = 418
NO_MODEL_ERROR_MESSAGE = "Error 418; No hay modelo disponible"

PREFERRED_PORTS = [8000, 8080, 3000, 5000]

# Memoria de conversación (horas antes de borrar el archivo de contexto)
# Si se pone en 0, el contexto nunca expirará automáticamente y se sobrescribirá.
CONTEXT_EXPIRATION_HOURS = 12

def find_free_port(candidates: list[int]) -> int:
    for port in candidates:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.bind(("0.0.0.0", port))
                return port
            except OSError:
                continue
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("0.0.0.0", 0))
        return s.getsockname()[1]
