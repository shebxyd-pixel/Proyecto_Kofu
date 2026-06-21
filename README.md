# Kofu v0.7 (Beta)

⚠️ **Estado**: En proceso de desarrollo (Beta v0.7) - Usa LLM de Ollama con Gemma 4 para razonamiento y sanitización. La IA en esta beta no es autónoma y puede fallar debido a errores de direcciones.

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

1. **Inicia el servidor**: Haz doble clic en `iniciar.bat` o ejecuta `cd backend && py server.py`
2. **Abre la interfaz**: Abre el archivo `web/index.html` en tu navegador
3. **Comienza a usar Kofu**: Escribe tus mensajes y crea documentos o presentaciones

## Funcionalidades principales

- **Chat interactivo**: Habla con Kofu
- **Crear documentos Word**: Genera documentos automáticos
- **Crear presentaciones PowerPoint**: Genera presentaciones automáticas
- **Modo Local**: Usa Kofu sin conexión a internet
- **Modo Online**: Investigación web y funciones avanzadas
- **Plantillas personalizadas**: Usa tus propias plantillas de Office
- **Carga de archivos**: Procesa documentos, imágenes, audio y más usando MarkItDown
- **Razonamiento offline avanzado**: Integración con Ollama para usar modelos locales

## AI Status

- La IA en esta beta no es autónoma.
- El sistema usa Ollama con Gemma 4 para razonamiento y sanitización local.
- Puede presentar errores de dirección y no debe considerarse una IA completamente autónoma en esta versión.
- En esta etapa, Kofu actúa como plataforma asistida por modelo local.

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

- Versión actual (0.7): Usa LLM Gemma 4.
- Uso: razonamiento, sanitización de código y procesamiento local sin conexión.
- Cómo configurar Ollama:
  1. Descarga e instala Ollama desde https://ollama.ai/
  2. Descarga un modelo compatible.
  3. Configura el archivo `.env` con `USE_OLLAMA=true`.
  4. Inicia Kofu con `iniciar.bat`.
- Beneficio: permite trabajar en modo local con un modelo de razonamiento avanzado.
- Licencias de terceros: los usuarios deben respetar los términos de licencia de Ollama.

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

### Configuración avanzada

MarkItDown está incluido en el proyecto en modo editable desde la carpeta `markitdown/`. Para más información sobre MarkItDown, consulta `markitdown/README.md`.

## Integración con Ollama (Razonamiento offline)

Kofu tiene integración con Ollama para usar modelos locales y razonamiento avanzado sin conexión a internet.

### Cómo configurar Ollama:
1. Descarga e instala Ollama desde [ollama.com/download](https://ollama.com/download)
2. Descarga un modelo (recomendado: `ollama pull llama3`)
3. Configura tu archivo .env con: USE_OLLAMA=true
4. Inicia Kofu con `iniciar.bat`

## Primeros pasos

1. Selecciona el tipo de documento (Word o PowerPoint)
2. (Opcional) Selecciona una plantilla
3. (Opcional) Carga un archivo para usar su contenido
4. Escribe tu solicitud y envía
5. Kofu creará el documento o presentación automáticamente

## Licencia y Términos de Uso

Kofu está sujeto a términos específicos y a la licencia contenida en el archivo `LICENSE`.

Resumen rápido:

- ✅ Uso personal: versiones posteriores a 1.2 permitidas.
- ✅ Uso educativo: versiones anteriores a 1.2 o versiones especiales educativas permitidas.
- ❌ No se permite usar la versión normal superior a 1.2 en contextos educativos.
- ❌ Uso comercial con este programa tal cual está no está permitido.
- ✅ Para uso comercial: solo versiones inferiores a 1.2 con modificación de código mínima del 80%, o 1.0 con modificaciones mínimas para funcionalidad.
- ✅ Proyectos derivados deben usar nombres DISTINTOS y no pueden llamarse "Kofu".

Para información completa, consulta el archivo `LICENSE`.

## Para más información

Consulta el `Manual de Usuario.md` para instrucciones detalladas y solución de problemas.

## Versiones Futuras y Planes

| Versión | Uso Personal | Uso Educativo | Uso Comercial |
|---|---|---|---|
| 0.7 Beta | ✅ Permitido localmente | ✅ Permitido con atribución | ❌ Restringido, requiere licencia o reestructura ≥80% |
| 1.0 Stable | ✅ Permitido | ✅ Permitido con logos y atribución | ❌ Solo con modificaciones mínimas para funcionalidad |
| 1.2 Stable y Optimized | ✅ Permitido | ✅ Solo versiones educativas especiales | ✅ Limitado bajo licencia |

### Roadmap por versión

- **0.7 Beta**
  - Versión actual que usa Ollama con Gemma 4 para razonamiento y sanitización.
  - La IA no es autónoma en beta y puede fallar por errores de direcciones.
  - Incluye MarkItDown para OCR y conversión de archivos.
  - Disponible para uso personal, educativo (con atribución) y comercial restringido.

- **1.0 Stable**
  - Primera versión estable del proyecto.
  - Versión autónoma con capacidad estable.
  - Mantenimiento y soporte continuo.
  - Uso educativo permitido hasta v1.0 con logos y atribución.
  - Uso comercial solo con modificaciones mínimas para funcionalidad.

- **1.2 Stable and Optimized**
  - Versión de largo plazo con optimizaciones y mayor robustez.
  - Enfocada en rapidez y rendimiento.
  - Uso comercial limitado disponible bajo licencia.
  - Versiones educativas especiales con integración a redes escolares y controles.

### Versiones educativas v1.2+

- Integración con redes escolares.
- Controles para maestros y administradores.
- Limitación de uso de estudiantes en modo local y online.
- Privacidad y seguridad de datos escolares.
- No se permite usar la versión normal superior a 1.2 en contextos educativos; solo versiones educativas especiales.

### Versiones comerciales v1.2+

- Basadas en la versión 1.2.
- Licencias comerciales limitadas.
- Soporte profesional y acuerdos personalizados.
- Optimizaciones empresariales para entornos corporativos.

### Cómo contribuir

- Si modificas más del 50% del código o agregas archivos para mejorar rendimiento,
  tendrás mayor libertad de edición y distribución.
- Asegúrate de documentar los cambios y mantener la atribución al origen.
