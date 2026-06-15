# LarI.A v0.4 (Beta) - Plataforma de Inteligencia Artificial para Microsoft Office

## Descripción General

LarIA es una plataforma de Inteligencia Artificial diseñada para automatizar la creación de documentos y presentaciones de Microsoft Word y PowerPoint. Cuenta con capacidades de razonamiento, corrección de errores de escritura, investigación web y uso de plantillas personalizadas.

---

## Estructura del Proyecto

```
webtrest/
├── web/                    # Interfaz web (HTML, CSS, JavaScript)
│   ├── index.html
│   ├── style.css
│   └── script.js
├── backend/                # Servidor y código de IA
│   ├── server.py           # Servidor Flask con API REST
│   └── src/                # Módulos de IA
│       ├── ai_assistant.py         # Asistente de IA completo
│       ├── office_agent.py         # Agente para crear documentos Office
│       ├── web_researcher.py       # Investigación en internet
│       ├── reasoning_engine.py     # Motor de razonamiento lógico
│       ├── external_ai_api.py      # Integración con APIs externas (Google, OpenAI)
│       ├── typo_corrector.py       # Corrección de errores de escritura
│       └── mcp_integration.py      # Integración con el protocolo MCP
├── templates/              # Plantillas de Office
│   ├── powerpoint/         # Plantillas de PowerPoint (.potx/.pptx)
│   └── word/               # Plantillas de Word (.dotx/.docx)
├── output/                 # Archivos generados (creado automáticamente)
├── vba_examples/           # Ejemplos de código VBA
├── Private/                # Archivos archivados (no necesarios)
├── requirements.txt        # Dependencias de Python
├── .env                    # Variables de entorno
└── iniciar.bat             # Script de inicio rápido
```

---

## Requisitos del Sistema

- **Sistema Operativo**: Windows 10 o superior
- **Python**: Versión 3.8 o superior
- **Microsoft Office**: PowerPoint y Word instalados
- **Conexión a Internet**: Para investigación web

---

## Instalación

### Paso 1: Instalar Python
Descarga e instala Python desde [python.org](https://www.python.org/) y marca la opción **"Add Python to PATH"** durante la instalación.

### Paso 2: Instalar dependencias
Abre una terminal en la carpeta del proyecto y ejecuta:

```bash
pip install -r requirements.txt
```

---

## Uso Rápido

### Opción 1: Script de inicio (Recomendado)
Haz doble clic en el archivo `iniciar.bat`.

### Opción 2: Terminal
1. Abre una terminal en la carpeta del proyecto
2. Ejecuta:
   ```bash
   cd backend
   py server.py
   ```

### Abrir la interfaz web
Abre el archivo `web/index.html` en tu navegador web.

---

## Funcionalidades Principales

### 1. Chat con la IA
- Interfaz de conversación intuitiva
- Corrección automática de errores de escritura
- Razonamiento paso a paso (Chain of Thought)

### 2. Crear Documentos de Word
- Usa plantillas personalizadas
- Estilos predefinidos (formato APA incluido)
- Generación automática de contenido

### 3. Crear Presentaciones de PowerPoint
- Temas de colores (professional, modern, vibrant)
- Plantillas personalizadas
- Diseño automático de slides

### 4. Investigación Web
- Búsqueda en DuckDuckGo (sin API key)
- Búsqueda en Google Custom Search (con API key)
- Extracción de contenido de páginas web
- Generación de resúmenes estructurados

---

## Uso de Plantillas

### Agregar una plantilla
1. **Crea tu plantilla** en PowerPoint o Word
2. **Guárdala** en la carpeta correspondiente:
   - PowerPoint: `templates/powerpoint/`
   - Word: `templates/word/`
3. **Formatos válidos**:
   - PowerPoint: `.potx` (recomendado), `.pptx`, `.pptm`
   - Word: `.dotx` (recomendado), `.docx`, `.docm`

### Usar una plantilla en la interfaz
1. Selecciona el **Tipo de documento** (Word/PowerPoint)
2. Selecciona la **Plantilla** que quieras usar
3. Escribe tu solicitud y envía

---

## Variables de Entorno

Crea un archivo `.env` en la carpeta raíz (usa `.env.example` como plantilla):

```env
# Google Custom Search (opcional)
GOOGLE_API_KEY=tu_clave_aqui
GOOGLE_CX=tu_cx_aqui

# OpenAI (opcional)
OPENAI_API_KEY=tu_clave_aqui
```

---

## API REST

El servidor cuenta con una API REST en `http://localhost:5000/api`:

| Endpoint | Método | Descripción |
|----------|--------|-------------|
| `/chat` | POST | Enviar mensaje a la IA |
| `/templates` | GET | Obtener lista de plantillas |
| `/create-document` | POST | Crear documento de Word |
| `/create-powerpoint` | POST | Crear presentación de PowerPoint |
| `/research` | POST | Investigar un tema |
| `/correct-typo` | POST | Corregir errores de escritura |
| `/external-ai` | POST | Consultar IA externa |
| `/office-tips` | GET | Obtener consejos de Office |

---

## Solución de Problemas

### Error: Faltan dependencias / No module named 'xxx'
El servidor mostrará un mensaje claro si falta alguna librería. Para solucionarlo:
1. Abre una terminal en la carpeta del proyecto
2. Ejecuta:
   ```bash
   pip install -r requirements.txt
   ```
3. Espera a que se instalen todas las dependencias
4. Vuelve a ejecutar el servidor

Si tienes problemas de permisos, prueba:
```bash
pip install --user -r requirements.txt
```

### No se conecta al servidor
- Asegúrate de que el servidor esté ejecutándose
- Verifica que el puerto 5000 esté disponible
- Asegúrate de haber instalado todas las dependencias (punto anterior)

### El iniciar.bat se abre y se cierra inmediatamente
1. Abre una terminal manualmente
2. Navega a la carpeta del proyecto
3. Ejecuta `iniciar.bat` desde la terminal para ver los mensajes de error
4. Asegúrate de haber instalado las dependencias

### No aparecen las plantillas
- Coloca las plantillas en la carpeta correcta (`templates/word/` o `templates/powerpoint/`)
- Asegúrate de que el formato sea válido

### Error al crear documentos
- Verifica que Microsoft Office esté instalado
- Asegúrate de que las rutas no tengan caracteres especiales

---

## Soporte
Para consultas o problemas, revisa los archivos en la carpeta `Private/` o contacta al equipo de desarrollo.

---

## Licencia
Proyecto de uso interno.
