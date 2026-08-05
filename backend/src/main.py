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

def run_api_server(port):
    uvicorn.run("api:app", host="0.0.0.0", port=port, log_level="error")

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
    print("   LarIA v1.0 - Servidor de IA Local")
    print("========================================")
    
    start_ollama()
    
    ports = [4010, 5050, 7070, 2607, 2005]
    processes = []
    
    print("Levantando servicios en puertos separados (delegado a Python):")
    for port in ports:
        p = multiprocessing.Process(target=run_api_server, args=(port,))
        p.start()
        processes.append(p)
        print(f" - Servicio backend en puerto {port}")
        
    print(" - Servidor Frontend en puerto 8000")
    
    # Configure Web Server
    app_web = FastAPI(title="LarIA Web")
    web_dir = os.path.join(os.path.dirname(__file__), '..', '..', 'web')
    
    app_web.mount("/web", StaticFiles(directory=web_dir), name="web")
    
    @app_web.get("/")
    def read_root():
        return RedirectResponse(url="/web/index.html")

    threading.Thread(target=open_browser, daemon=True).start()
    
    print("\nTodos los servicios iniciados correctamente. Presiona CTRL+C para detener.")
    
    try:
        uvicorn.run(app_web, host="0.0.0.0", port=8000, log_level="warning")
    except KeyboardInterrupt:
        print("\nDeteniendo servicios...")
    finally:
        for p in processes:
            p.terminate()
            p.join()
        print("Servidores detenidos.")

if __name__ == "__main__":
    multiprocessing.freeze_support()
    main()
