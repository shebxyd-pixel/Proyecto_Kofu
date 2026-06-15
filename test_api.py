import requests
import time
import json

print("="*80)
print("INICIANDO PRUEBA FUNCIONAL")
print("="*80)

# --------------------------
# TEST 1: /api/chat endpoint (con prompt EXACTO solicitado)
# --------------------------
URL_CHAT = "http://localhost:5000/api/chat"
PROMPT_EXACTO = "Crea una presentacion de powerpoint sobre comunidades cirtuales"
print(f"\n--- TEST 1: /api/chat ---")
print(f"1.1 Enviando prompt exacto: {PROMPT_EXACTO}")

test_data_chat = {
    "message": PROMPT_EXACTO,
    "show_thinking": True
}

try:
    response_chat = requests.post(URL_CHAT, json=test_data_chat, timeout=30)
    print("\n1.2 RESPUESTA RECIBIDA (Chat):")
    print(f"   - Código de estado HTTP: {response_chat.status_code}")
    print(f"\n   - Cabeceras:")
    for k, v in response_chat.headers.items():
        print(f"     {k}: {v}")
    print(f"\n   - Cuerpo bruto:")
    print("     " + response_chat.text)
    print("\n   - Cuerpo JSON:")
    print(json.dumps(response_chat.json(), indent=4))
    
except Exception as e:
    print("\nERROR en TEST 1:")
    print(f"  Tipo: {type(e).__name__}")
    print(f"  Mensaje: {str(e)}")

# --------------------------
# TEST 2: /api/create-powerpoint endpoint
# --------------------------
URL_PPT = "http://localhost:5000/api/create-powerpoint"
print(f"\n--- TEST 2: /api/create-powerpoint ---")

test_data_ppt = {
    "topic": "Comunidades cirtuales",
    "theme": "professional",
    "filename": "presentacion_comunidades_cirtuales.pptx"
}

print(f"\n2.1 Enviando solicitud a {URL_PPT}")
print(f"   Datos: {json.dumps(test_data_ppt, indent=4)}")

try:
    response_ppt = requests.post(URL_PPT, json=test_data_ppt, timeout=30)
    print("\n2.2 RESPUESTA RECIBIDA (PPT):")
    print(f"   - Código de estado HTTP: {response_ppt.status_code}")
    print(f"\n   - Cabeceras:")
    for k, v in response_ppt.headers.items():
        print(f"     {k}: {v}")
    print(f"\n   - Cuerpo bruto:")
    print("     " + response_ppt.text)
    print("\n   - Cuerpo JSON:")
    print(json.dumps(response_ppt.json(), indent=4))
    
except Exception as e:
    print("\nERROR en TEST 2:")
    print(f"  Tipo: {type(e).__name__}")
    print(f"  Mensaje: {str(e)}")

print("\n" + "="*80)
print("FIN DE LA PRUEBA")
print("="*80)
