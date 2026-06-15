
# Webtrest v2.0 - Fase 1: LLM Local + RAG

## 🚀 Instalación Rápida

### 1. Requisitos Previos
- **Python 3.11 o superior**
- **Git** (opcional)
- **GPU NVIDIA recomendada** (con CUDA 11.8+ para mejor rendimiento)
- 16 GB RAM mínimo, 32 GB recomendado

### 2. Instalar Dependencias
```bash
cd c:\Users\User\Desktop\laliza\webtrest
pip install -r requirements.txt
```

### 3. Ejecutar el Servidor
Doble-click en **`iniciar.bat`** y selecciona la opción **2** (Servidor v2.0).

O desde línea de comandos:
```bash
python server_v2.py
```

### 4. Usar el Chatbot
Abre **`index.html`** en tu navegador.

---

## 📋 Archivos Nuevos de la Fase 1

| Archivo | Descripción |
|---------|-------------|
| `src/llm_engine.py` | Motor LLM local (Mistral 7B) con QLoRA |
| `src/rag_system.py` | Sistema RAG con Chroma DB |
| `src/ai_assistant_v2.py` | Asistente actualizado |
| `server_v2.py` | Servidor Flask v2 |
| `.env.example` | Plantilla de configuración |
| `requirements.txt` | Dependencias actualizadas |
| `iniciar.bat` | Script de inicio mejorado |

---

## 💡 Uso del Sistema RAG

Para agregar tus propios documentos a la base de conocimientos:
1. Coloca archivos `.txt`, `.pdf` o `.docx` en una carpeta
2. Usa la API `/api/add-knowledge`

---

## 🔧 Modelos Soportados

- **Predeterminado**: `mistralai/Mistral-7B-Instruct-v0.3` (7B parámetros)
- **Alternativas**:
  - `meta-llama/Llama-3.1-8B-Instruct`
  - `Qwen/Qwen2.5-7B-Instruct`

Para cambiar el modelo, edita el archivo `.env` y modifica `LLM_MODEL_NAME`.

---

## ⚙️ Optimizaciones para GPU

Si tienes GPU NVIDIA, instala PyTorch con CUDA:
```bash
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121
```

---

## ❓ Preguntas Frecuentes

**¿Funciona sin GPU?**  
Sí, pero será muy lento. Usa CPU solo para pruebas.

**¿Dónde se guardan los documentos del RAG?**  
En `./data/chroma_db/`

**¿Puedo usar el servidor original?**  
Sí, selecciona opción 1 en `iniciar.bat`.
