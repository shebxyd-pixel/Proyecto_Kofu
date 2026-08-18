import os
import sys
import subprocess
import time
import multiprocessing
import uvicorn
import webbrowser
import threading
from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles
from fastapi.responses import RedirectResponse
from api import app

def start_ollama():
    try:
        import urllib.request
        try:
            urllib.request.urlopen("http://localhost:11434", timeout=1)
        except Exception:
            print("Iniciando servicio nativo de Ollama en segundo plano...")
            subprocess.Popen(["ollama", "serve"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            time.sleep(2)
    except Exception:
        pass

def open_browser():
    time.sleep(2)
    print("Abriendo navegador en http://localhost:8000/")
    webbrowser.open("http://localhost:8000/")

def main():
    print("========================================")
    print("   Kofu v1.0 (Beta) - Servidor de IA")
    print("========================================")
    
    start_ollama()
    
    print("Levantando servicio unificado (Backend + Frontend) en puerto 8000...")
    
    # Configure Web Server and API
    from paths import WEB_DIR
    web_dir = WEB_DIR
    app.mount("/web", StaticFiles(directory=web_dir), name="web")
    
    @app.get("/")
    def read_root():
        return RedirectResponse(url="/web/index.html")

    threading.Thread(target=open_browser, daemon=True).start()
    
    print("\nTodos los servicios iniciados correctamente en el puerto 8000. Presiona CTRL+C para detener.")
    
    try:
        uvicorn.run(app, host="0.0.0.0", port=8000, log_level="warning", ws="none")
    except KeyboardInterrupt:
        print("\nDeteniendo servicios...")
    finally:
        print("Servidor detenido.")

if __name__ == "__main__":
    multiprocessing.freeze_support()
    main()
