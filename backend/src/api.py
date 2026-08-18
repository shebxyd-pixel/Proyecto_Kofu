import os

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from config import find_free_port, PREFERRED_PORTS
from routes import router

app = FastAPI(title="Kofu API", version="0.8.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(router)

if __name__ == "__main__":
    import uvicorn

    port = find_free_port(PREFERRED_PORTS)

    from paths import WEB_DIR
    port_file = os.path.join(WEB_DIR, '.port')
    try:
        with open(port_file, 'w') as f:
            f.write(str(port))
    except Exception:
        pass

    print(f"Servidor Kofu iniciado en http://localhost:{port}")
    # uvicorn.run(app, host="0.0.0.0", port=port)
