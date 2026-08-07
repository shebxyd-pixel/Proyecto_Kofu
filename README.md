# Kofu v1.0 (Latest)

**Asistente autonomo multiplataforma 100% offline** de creacion de Documentos '.docx' y '.potx' usando diversas plantillas sin necesidad de tener Office u LibreOffice **usando el motor ollama para el procesamiento de ordenes e insercion de contexto hibrido.** 

## Requisitos del sistema

- **Sistema Operativo**: Windows 8.1/10/11, Linux, o macOS en sus versiones x64. 
- **Python**: Versión 3.10 o superior (recomendado 3.10 o 3.11).
- **Ofimática (Office / LibreOffice)**:
  - **Para generar documentos**: NO es necesario tener instalado Microsoft Office ni LibreOffice. Kofu genera nativamente los archivos `.docx` y `.pptx` de manera independiente usando bibliotecas de Python (`python-docx` y `python-pptx`).
  - **Para abrir y visualizar los documentos**: Necesitarás **Microsoft Office** (Word/PowerPoint), **LibreOffice** (Writer/Impress), u otra suite ofimática compatible con los formatos de OpenXML.
- **Procesamiento de Audio / MarkItDown**: Si deseas procesar archivos de audio (MP3, WAV, etc.), es necesario tener instalado **[FFmpeg](https://ffmpeg.org/)** en tu sistema y agregado al PATH (requerido por `pydub`).
- **Hardware (Para uso local)**: Mínimo 8 GB de RAM (16 GB o más recomendados si utilizas modelos locales de Ollama). Una GPU dedicada (NVIDIA/AMD) acelerará enormemente el procesamiento local, aunque puede funcionar en CPU (más lento).
- **Conexión a Internet**: Necesaria para la instalación inicial, y requerida para el modo Online (investigación web o procesamiento externo de IA).

## Instalación

### Python

1. Instala Python desde [python.org](https://www.python.org/)
2. Abre una terminal en la carpeta del proyecto
3. Ejecuta: `pip install -r requirements.txt`


1. Instala ollama en **[Ollama](https://ollama.com/)**
#### Descarga los 3 modelos recomendados.

  - Para conversar:
    `ollama pull qwen2.5:0.5b-instruct` o `ollama pull smollm2:360m`
  - Para generar documentos:
    `ollama pull gemma` o `ollama pull llama3`

## Uso rápido

1. **Inicia el servidor**: Ejecuta el archivo de arranque correspondiente (ej. `run.bat`, `run.sh`, `run.ps1`, o `run_macos.command`) En sistemas linux 'run.sh' [Solo se ha testeado en versiones basadas en ubuntu 25]
2. **Descarga automatica**: Al arrancar el servidor por primera vez creara un entorno virtual e instalara las dependencias necesarias, esto puede tardar un poco la primera vez.
3. **Comienza a usar Kofu**: Al terminar la instalacion se abrira automaticamente la interfaz de Kofu, si no se abre entonces puedes ir a 'localhost:8000'

## Funcionalidades principales

- **Chat interactivo**: Habla con Kofu
- **Crear documentos Word**: Genera documentos automáticos
- **Crear presentaciones PowerPoint**: Genera presentaciones automáticas
- **Modo Local**: Usa Kofu sin conexión a internet
- **Modo Online**: Investigación web y funciones avanzadas
- **Plantillas personalizadas**: Usa tus propias plantillas de Office o las integradas por defecto
- **Carga de archivos**: Procesa documentos, imágenes, audio y más usando MarkItDown [Experimental]
- **Razonamiento offline avanzado**: Integración con el motor Ollama para usar modelos de forma local

## AI Status

- La IA en esta version v1.0 estable y lista para usar.
- El backend usa el motor Ollama para procesar la informacion y peticiones.
- No se ha habillitado la version 100% local debido a conflictos con 'markitdown' y subdependencias
- Kofu actua de manera independiente sin requerir usar office ni aplicaciones externas

## Stack Tecnológico

### MarkItDown

Kofu utiliza **MarkItDown** como una capa de conversión y extracción de contenido.

- Qué es: Repositorio de Microsoft para OCR y conversión de archivos a Markdown.
- Uso en Kofu: Procesa entradas como PDF, DOCX, PPTX, imágenes y audio para su integración en el flujo de generación.
- Formatos soportados:
  - PDF (.pdf)
  - Word (.docx)
  - PowerPoint (.pptx)
  - Excel (.xlsx, .xls)
  - Imágenes (.jpg, .jpeg, .png, .gif, .bmp, .tiff)
  - Audio (.wav, .mp3, .m4a, .ogg, .flac)
  - HTML, Markdown, texto plano, CSV, JSON, XML
  - EPUB, ZIP, Outlook (.msg), Jupyter Notebooks (.ipynb)
- Cómo se usa en Kofu:
  - Convertir archivos cargados a texto/Markdown para su análisis por el motor de IA.
  - Extraer contenido útil de documentos y presentaciones antes de generar resultados.
- Seguridad:
  - El procesamiento se realiza con restricciones del proceso actual.
  - Kofu sanitiza las entradas antes de su uso para reducir riesgos.
- Enlace: https://github.com/microsoft/markitdown

### Ollama (Razonamiento offline)

Kofu integra **Ollama** para razonamiento local y sanitización de código.

- Versión actual (1.0): Usa varios modelos de manera dinamica
- Uso: razonamiento, sanitización de código y procesamiento local sin conexión.
- Cómo configurar Ollama:
  1. Descarga e instala Ollama desde https://ollama.ai/
  2. Descarga un modelo compatible o los recomendados.
  4. Inicia Kofu con tu script de arranque (ej. `run.bat`).

- Licencias de terceros: los usuarios deben respetar los términos de licencia de Ollama.

### Configuración avanzada
MarkItDown está integrado directamente en el proyecto en el archivo `backend/src/markitdown.py`.

## Integración con MarkItDown

Kofu usa **MarkItDown** para convertir diversos tipos de archivos a Markdown/texto para su procesamiento por modelos de lenguaje.

### Funcionalidades habilitadas por MarkItDown

- **Formatos soportados:**
  - PDF (.pdf)
  - Documentos de Word (.docx)
  - Presentaciones de PowerPoint (.pptx)
  - Hojas de cálculo Excel (.xlsx, .xls)
  - Imágenes (.jpg, .jpeg, .png, .gif, .bmp, .tiff)
  - Audio (.wav, .mp3, .m4a, .ogg, .flac)
  - HTML, texto plano, Markdown, CSV, JSON, XML
  - EPUB, ZIP, Outlook (.msg), Jupyter Notebooks (.ipynb)

- **Uso:**
  - Arrastra y suelta un archivo en la interfaz de Kofu.
  - Selecciona un documento o presentación para generar.
  - Kofu extraerá el contenido del archivo y podrás usarlo para crear nuevos documentos o presentaciones.

- **Seguridad:**
  - MarkItDown procesa los archivos con las restricciones del proceso actual.
  - Kofu sanitiza todas las entradas antes de procesarlas para garantizar seguridad.

### Cómo configurar Ollama:
1. Descarga e instala Ollama desde [ollama.com/download](https://ollama.com/download)
2. Descarga un modelo (recomendado: `ollama pull llama3`)
4. Inicia Kofu con tu script de arranque (ej. `run.bat`).

## Primeros pasos

1. Selecciona el tipo de documento (Word o PowerPoint)
2. Selecciona una plantilla
  - (Opcional) Carga un archivo para usar su contenido
3. Escribe tu solicitud y envía
4. Kofu creará el documento o presentación automáticamente

## Licencia y Términos de Uso

Kofu es un proyecto de código abierto (Open Source) sujeto a la licencia contenida en el archivo `LICENSE`.

Resumen rápido:

- **Inspiración y aprendizaje**: Eres libre de usar el código para aprender y usarlo como referencia.
- **No plagio**: Queda prohibido apropiarse del código, copiarlo y afirmar que es tuyo (no lo robes).
- **Actualizaciones**: El proyecto recibirá parches y modificaciones de forma aleatoria, sin un calendario regular o periódico.
- **Terceros**: Kofu integra MarkItDown (Microsoft) y el motor Ollama, ambos con sus respectivas licencias MIT.

Para información completa, consulta el archivo `LICENSE`.

### Roadmap por versión

- **0.3 Estable**
  - Generacion de documentos simples forzando conexiones estables por consola

- **0.5 Estable**
  - integracion de Web Reasearch

- **0.6 Estable**
  - Agregar modulos de sanitizacion
  - Agregar modulos compatibles para api
  - Mejora de Frontend

- **0.7 Beta**
  - Versión actual que usa Ollama con Gemma 4 para razonamiento y sanitización.
  - La IA no es autónoma en beta y puede fallar por errores de direcciones.
  - Incluye MarkItDown para OCR y conversión de archivos.
  - Disponible para uso personal, educativo (con atribución) y comercial restringido.

- **0.8 Estable**
  - Resolucion de conflictos Cors
  - Primer resolucion de problemas complejos.
  - Solucion de creacion de archivos
  - Solucion con archivos Markitdown crudos

- **0.9 Beta**
  - Comunicacion estable
  - Uso de Uvicorn
  - Separacion del proyecto a LarIA(0.5)
  - Creacion de plantillas

- **1.0 Stable** _(Latest)_
  - Primera versión estable del proyecto.
  - Versión autónoma con capacidad estable.
  - Primer version Funcional
  - Independencia de Office
  - Uso de plantillas
  - Integracion de contexto inyectado
  - Uso de diversos motores de razonamiento de forma dinamica


### Cómo contribuir

- Sigue los normamientos de github y no te apropies ni robes el codigo
