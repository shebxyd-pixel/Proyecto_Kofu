# LarIA v0.4 (Beta)

⚠️ **Estado**: En proceso de desarrollo (Beta v0.4) - Usa LLM de Ollama con Gemma 4 para razonamiento y sanitización.

Plataforma de Inteligencia Artificial para Microsoft Office.

## Requisitos del sistema

- **Sistema Operativo**: Windows 10 o superior
- **Python**: Versión 3.10 o superior
- **Microsoft Office**: PowerPoint y Word instalados
- **Conexión a Internet**: Para investigación web (modo Online)

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

- Versión actual (0.4): Usa LLM Gemma 4.
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
