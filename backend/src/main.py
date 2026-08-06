import os
import time
import subprocess
import uvicorn
import webbrowser
import threading
from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles
from fastapi.responses import RedirectResponse
from fastapi.middleware.cors import CORSMiddleware
from routes import router

def start_ollama():
    try:
        import urllib.request
        urllib.request.urlopen("http://localhost:11434", timeout=1)
    except Exception:
        print("Iniciando servicio nativo de Ollama en segundo plano...")
        subprocess.Popen(["ollama", "serve"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(1)

def open_browser():
    time.sleep(2)
    print("Abriendo navegador en http://localhost:8000/")
    webbrowser.open("http://localhost:8000/")

def main():
    print("========================================")
    print("   LarIA v0.5 (Beta) - Servidor Ligero")
    print("========================================")
    
    start_ollama()
    
    app_web = FastAPI(title="LarIA Web y API")
    
    app_web.add_middleware(
        CORSMiddleware,
        allow_origins=["*"],
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    )

    # El backend y el frontend comparten el mismo puerto ahora para ser absurdo y ligero
    app_web.include_router(router)
    
    web_dir = os.path.join(os.path.dirname(__file__), '..', '..', 'web')
    app_web.mount("/web", StaticFiles(directory=web_dir), name="web")
    
    @app_web.get("/")
    def read_root():
        return RedirectResponse(url="/web/index.html")

    threading.Thread(target=open_browser, daemon=True).start()
    
    print("\nLarIA ultra-ligera iniciada. Presiona CTRL+C para detener.")
    uvicorn.run(app_web, host="0.0.0.0", port=8000, log_level="warning")

if __name__ == "__main__":
    main()
