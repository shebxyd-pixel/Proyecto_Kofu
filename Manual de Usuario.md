# Manual de Usuario de Kofu v0.7 (Beta)

## Índice

1. [Introducción](#1-introducción)
2. [Instalación y configuración](#2-instalación-y-configuración)
3. [Uso básico](#3-uso-básico)
4. [Modos de operación](#4-modos-de-operación)
5. [Plantillas personalizadas](#5-plantillas-personalizadas)
6. [Solución de problemas](#6-solución-de-problemas)

---

## 1. Introducción

Kofu es una plataforma de Inteligencia Artificial diseñada para automatizar la creación de documentos y presentaciones de Microsoft Office. Con Kofu, puedes:

- Chatear y preguntar sobre diversos temas
- Generar documentos de Word automáticamente
- Generar presentaciones de PowerPoint automáticamente
- Investigar temas en internet
- Usar tus propias plantillas personalizadas

---

## 2. Instalación y configuración

### 2.1 Requisitos del sistema

Antes de instalar Kofu, asegúrate de tener:

- **Sistema Operativo**: Windows 10 o superior
- **Python**: Versión 3.8 o superior (descarga desde [python.org](https://www.python.org/))
- **Microsoft Office**: PowerPoint y Word instalados
- **Conexión a Internet**: Para usar el modo Online y la investigación web

### 2.2 Instalación de Python

1. Ve a [python.org/downloads](https://www.python.org/downloads/)
2. Descarga la última versión de Python
3. Durante la instalación, **asegúrate de marcar la opción "Add Python to PATH"**
4. Completa la instalación

### 2.3 Instalación de dependencias

1. Abre una terminal (Command Prompt o PowerShell)
2. Navega a la carpeta del proyecto Kofu
3. Ejecuta el siguiente comando:

   ```bash
   pip install -r requirements.txt
   ```

4. Espera a que se instalen todas las dependencias

### 2.4 Variables de entorno (opcional)

Si quieres usar APIs personalizadas, puedes crear un archivo `.env` en la carpeta raíz (usa `.env.example` como plantilla).

---

## 3. Uso básico

### 3.1 Iniciar Kofu

Hay dos formas de iniciar Kofu:

#### Opción 1: Usar el archivo de inicio (recomendado)
1. Haz doble clic en el archivo `iniciar.bat`
2. Espera a que se inicie el servidor

#### Opción 2: Usar la terminal
1. Abre una terminal
2. Navega a la carpeta del proyecto
3. Ejecuta:

   ```bash
   cd backend
   py server.py
   ```

### 3.2 Abrir la interfaz web

1. Abre tu navegador web (Chrome, Edge, Firefox, etc.)
2. Abre el archivo `web/index.html` (puedes hacer doble clic en él)
3. Verás la interfaz de Kofu

### 3.3 Interfaz principal

La interfaz de Kofu está dividida en dos partes:

- **Barra lateral izquierda**:
  - Nombre y versión de Kofu
  - Selector de modo (Local/Online)
  - Configuración de API Key (solo modo Online)
  - Contador de tareas activas
  - Botón de apagado

- **Área principal**:
  - Historial de chat
  - Opciones de documento y plantilla
  - Campo de texto para escribir mensajes

### 3.4 Crear tu primer documento o presentación

1. **Selecciona el tipo de documento**:
   - Haz clic en el menú "Tipo de documento"
   - Selecciona "Word" o "PowerPoint"

2. **(Opcional) Selecciona una plantilla**:
   - Si tienes plantillas personalizadas, aparecerán aquí
   - Selecciona la que quieras usar

3. **Escribe tu solicitud**:
   - En el campo de texto, escribe lo que quieres que Kofu haga
   - Ejemplo: "Crea una presentación sobre inteligencia artificial"
   - Ejemplo: "Haz un documento sobre el cambio climático"

4. **Envía el mensaje**:
   - Haz clic en el botón de enviar (icono de avión de papel)
   - O presiona Enter

5. **Espera la respuesta**:
   - Kofu creará el documento o presentación automáticamente
   - Los archivos se guardan en la carpeta `output/`

---

## 4. Modos de operación

Kofu tiene dos modos de operación:

### 4.1 Modo Local (predeterminado)

Características del modo Local:
- No requiere conexión a internet
- Crea documentos y presentaciones usando plantillas
- Usa el razonamiento integrado de Kofu
- Ideal para uso básico y offline

Para activar el modo Local:
1. Haz clic en el botón "Local" en la barra lateral
2. El botón se pondrá en azul (activo)

### 4.2 Modo Online

Características del modo Online:
- Requiere conexión a internet
- Realiza investigaciones web automáticas
- Busca información actualizada
- Necesita una API Key (dependiendo de la configuración)

Para activar el modo Online:
1. Haz clic en el botón "Online" en la barra lateral
2. El botón se pondrá en azul (activo)
3. Aparecerá un campo para la API Key
4. Ingresa tu API Key (se guarda automáticamente)

#### Investigación web en modo Online

Para investigar un tema:
1. Asegúrate de estar en modo Online
2. Escribe una solicitud de investigación
3. Ejemplos:
   - "Investiga sobre el Big Bang"
   - "Busca información sobre Python"
   - "Qué es la inteligencia artificial"
4. Kofu buscará información y te mostrará un resumen con fuentes

---

## 5. Plantillas personalizadas

Kofu te permite usar tus propias plantillas de Word y PowerPoint.

### 5.1 Tipos de archivos soportados

- **PowerPoint**: .pptx, .potx, .pptm
- **Word**: .docx, .dotx, .docm

### 5.2 Ubicación de las plantillas

Coloca tus plantillas en las carpetas correspondientes:
- Plantillas PowerPoint: `templates/powerpoint/`
- Plantillas Word: `templates/word/`

### 5.3 Cómo crear una plantilla

#### Para PowerPoint:
1. Abre PowerPoint
2. Crea una presentación con el diseño que quieras
3. Ve a Archivo → Guardar como
4. Selecciona el tipo "Plantilla PowerPoint (*.potx)"
5. Guarda el archivo
6. Copia el archivo a `templates/powerpoint/`

#### Para Word:
1. Abre Word
2. Crea un documento con el diseño que quieras
3. Ve a Archivo → Guardar como
4. Selecciona el tipo "Plantilla Word (*.dotx)"
5. Guarda el archivo
6. Copia el archivo a `templates/word/`

### 5.4 Ver tus plantillas en Kofu

1. Reinicia el servidor de Kofu (si estaba corriendo)
2. En la interfaz web, selecciona el tipo de documento
3. El menú "Plantilla" mostrará tus plantillas personalizadas
4. Selecciona la que quieras usar

---

## 6. Solución de problemas

### Error: "Faltan dependencias" o "No module named 'xxx'"

**Solución**:
1. Asegúrate de haber instalado todas las dependencias:
   ```bash
   pip install -r requirements.txt
   ```
2. Si sigue sin funcionar, intenta:
   ```bash
   pip install --user -r requirements.txt
   ```

### No se conecta al servidor

**Posibles causas y soluciones**:
1. Asegúrate de que el servidor esté corriendo (deberías ver mensajes en la terminal)
2. Verifica que el puerto 5000 esté disponible (cierra otros programas que lo estén usando)
3. Asegúrate de haber instalado todas las dependencias

### El archivo iniciar.bat se abre y se cierra inmediatamente

**Solución**:
1. Abre una terminal manualmente
2. Navega a la carpeta del proyecto
3. Ejecuta el archivo desde la terminal para ver los mensajes de error:
   ```bash
   iniciar.bat
   ```
4. Lee los mensajes de error para identificar el problema

### No aparecen mis plantillas

**Solución**:
1. Asegúrate de que las plantillas están en la carpeta correcta:
   - PowerPoint: `templates/powerpoint/`
   - Word: `templates/word/`
2. Verifica que el formato de la plantilla sea válido
3. Reinicia el servidor de Kofu

### Error al crear documentos o presentaciones

**Posibles causas y soluciones**:
1. Asegúrate de que Microsoft Office esté instalado correctamente
2. Verifica que las rutas no tengan caracteres especiales (ñ, tildes, espacios extraños)
3. Asegúrate de tener permisos para escribir en la carpeta `output/`

### Error: "La ia no esta lista para usar, Intenta de nuevo mas tarde o vea el manual de errores"

**Solución**:
1. Verifica que el servidor esté corriendo
2. Revisa la terminal del servidor para ver mensajes de error
3. Asegúrate de que todas las dependencias estén instaladas
4. Reinicia el servidor

### Modo Online no funciona

**Solución**:
1. Asegúrate de tener conexión a internet
2. Verifica que hayas ingresado una API Key válida (si es necesaria)
3. Revisa que la API Key esté correctamente escrita

### Preguntas frecuentes

#### ¿Puedo usar Kofu en Mac o Linux?
Actualmente Kofu está diseñado para Windows, ya que usa Microsoft Office.

#### ¿Los documentos generados se guardan automáticamente?
Sí, todos los documentos y presentaciones se guardan en la carpeta `output/`.

#### ¿Kofu guarda mis conversaciones?
No, las conversaciones no se guardan. Cada vez que actualices la página, el historial se borra.

#### ¿Puedo usar Kofu sin internet?
Sí, usando el modo Local.

#### ¿Cómo actualizo Kofu?
Descarga la versión más nueva y reemplaza los archivos (mantén tu carpeta `templates/` y `output/`).

---

## Soporte

Si tienes problemas o preguntas:
1. Consulta este manual primero
2. Verifica la sección de "Solución de problemas"
3. Revisa los mensajes en la terminal del servidor
