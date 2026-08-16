# LarIA v0.5 (Beta)

⚠️ **Estado**: En proceso de desarrollo (Beta v0.5) - Usa LLM de Ollama con Gemma 4 para razonamiento y sanitización.

Plataforma de Inteligencia Artificial para Microsoft Office.

**¿Qué es LarIA?**
LarIA es un proyecto experimental tipo "What if?" (¿Qué pasaría si...?) que nace de Kofu, pero está diseñado para ser absurdamente rápido y ligero. A diferencia de Kofu que es robusto y potente, LarIA sacrifica módulos no vitales (como múltiples puertos, seguridad pesada de sanitización o procesos complejos) con un único propósito: generar presentaciones y documentos a una velocidad ridículamente alta y con el menor consumo de recursos posible, manteniendo la misma calidad de texto generado.

## Requisitos del sistema

- **Sistema Operativo**: Windows 10/11, Linux o macOS
- **Procesador**: 4 núcleos mínimo (8 núcleos recomendado)
- **Memoria RAM**: 8 GB mínimo (16 GB recomendado para usar Ollama local)
- **Almacenamiento**: 10 GB libres para modelos de IA
- **Python**: Versión 3.9 a 3.11 (Asegúrate de marcar "Add Python to PATH" durante la instalación).
- **Ollama**: Necesario para el razonamiento local. [Descárgalo aquí](https://ollama.com).

## Guía de Instalación y Uso (Paso a Paso)

### 0. Clonar el repositorio 

'git clone -b LarIA_v0.4 --single-branch [https://github.com/TU_USUARIO/Proyecto_Kofu.git](https://github.com/TU_USUARIO/Proyecto_Kofu.git)'

'cd Proyecto_Kofu'

### 1. Instalar Python y Ollama
- Descarga e instala [Python](https://www.python.org/downloads/) (marca la opción "Add to PATH").
- Descarga e instala [Ollama](https://ollama.com/download). 
- *(Opcional)* Abre una terminal y verifica la instalación escribiendo: `ollama -v` y `python --version`.

### 2. Ejecutar LarIA
LarIA está diseñada para que no tengas que lidiar con la terminal si no quieres. Simplemente haz doble clic en el script correspondiente a tu sistema:

- **Windows:** Haz doble clic en `run.bat` o `run.ps1`.
- **Linux / macOS:** Abre la terminal, navega a la carpeta y ejecuta `./run.sh` (si pide permisos, usa `chmod +x run.sh` primero).

### 3. Asistente Interactivo de Modelos
Al ejecutar el script de inicio, LarIA revisará automáticamente:
1. Si tienes Python y creará un entorno virtual (`venv`) aislado.
2. Si te faltan dependencias, te preguntará si deseas instalarlas (responde `s`).
3. Verificará tu instalación de Ollama. Si no tienes modelos, **descargará automáticamente `llama3:latest`**.
4. Si ya tienes modelos instalados, te mostrará un menú dándote a elegir entre:
   - Descargar `llama3:latest` (recomendado).
   - Descargar `gemma4:latest` (segunda opción).
   - Omitir la descarga y usar los que ya tienes listos.

### 4. Listo para usar
Una vez completado el inicio, LarIA levantará un único servidor ultraligero y te abrirá automáticamente el navegador en:
`http://localhost:8000/`

Desde ahí podrás chatear, pedirle documentos Word o presentaciones PowerPoint directamente a la IA.

## Instalación

1. Instala Python desde [python.org](https://www.python.org/)
2. Abre una terminal en la carpeta del proyecto
3. Ejecuta: `pip install -r requirements.txt`

## Uso rápido

1. **Inicia el servidor**: Haz doble clic en `run.bat` o ejecuta `cd backend && py server.py`
2. **Abre la interfaz**: Abre el archivo `web/index.html` en tu navegador
3. **Comienza a usar LarIA**: Escribe tus mensajes y crea documentos o presentaciones

## Funcionalidades principales

- **Chat interactivo**: Habla con LarIA
- **Crear documentos Word**: Genera documentos automáticos
- **Crear presentaciones PowerPoint**: Genera presentaciones automáticas
- **Modo Local**: Usa LarIA sin conexión a internet
- **Modo Online**: Investigación web y funciones avanzadas
- **Plantillas personalizadas**: Usa tus propias plantillas de Office
- **Carga de archivos**: Procesa documentos, imágenes, audio y más usando MarkItDown
- **Razonamiento offline avanzado**: Integración con Ollama para usar modelos locales

## AI Status

- La IA en esta beta no es autónoma.
- El sistema usa Ollama con Gemma 4 para razonamiento y sanitización local.
- Puede presentar errores de dirección y no debe considerarse una IA completamente autónoma en esta versión.
- En esta etapa, LarIA actúa como plataforma asistida por modelo local.

## Stack Tecnológico

### MarkItDown

LarIA utiliza **MarkItDown** como una capa de conversión y extracción de contenido.

- Qué es: Repositorio de Microsoft para OCR y conversión de archivos a Markdown.
- Uso en LarIA: Procesa entradas como PDF, DOCX, PPTX, imágenes y audio para su integración en el flujo de generación.
- Seguridad:
  - El procesamiento se realiza con restricciones del proceso actual.
  - LarIA sanitiza las entradas antes de su uso para reducir riesgos.

### Ollama (Razonamiento offline)

LarIA integra **Ollama** para razonamiento local y sanitización de código.

- Versión actual (0.5): Usa LLM Gemma 4.
- Uso: razonamiento, sanitización de código y procesamiento local sin conexión.
- Beneficio: permite trabajar en modo local con un modelo de razonamiento avanzado.
- Licencias de terceros: los usuarios deben respetar los términos de licencia de Ollama.

## Integración con Ollama (Razonamiento offline)

LarIA tiene integración con Ollama para usar modelos locales y razonamiento avanzado sin conexión a internet.

### Cómo configurar Ollama:
1. Descarga e instala Ollama desde [ollama.com/download](https://ollama.com/download)
2. Descarga un modelo (recomendado: `ollama pull llama3`)
3. Configura tu archivo .env con: USE_OLLAMA=true
4. Inicia LarIA con `run.bat`

## Primeros pasos

1. Selecciona el tipo de documento (Word o PowerPoint)
2. (Opcional) Selecciona una plantilla
3. (Opcional) Carga un archivo para usar su contenido
4. Escribe tu solicitud y envía
5. LarIA creará el documento o presentación automáticamente

## Licencia y Términos de Uso

LarIA está sujeto a términos específicos y a la licencia contenida en el archivo `LICENSE`.

Para información completa, consulta el archivo `LICENSE`.
