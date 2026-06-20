# Resumen de errores y problemas identificados

Este archivo resume los fallos más importantes detectados en el proyecto y su relación con la carpeta `Funcionamiento`.

## 1. Fallos principales

- **F1 - Reglas insuficientes en el motor de razonamiento**
  - Archivo: `backend/src/reasoning_engine.py`
  - Detalle: No existen reglas generales para detectar solicitudes de creación de documentos o presentaciones fuera de pentesting.
  - Impacto: El chat no reconoce correctamente solicitudes como "crear presentación" o "crear documento" para temas generales.

- **F2 - `/api/chat` no integra la generación de archivos**
  - Archivo: `backend/server.py`
  - Detalle: El endpoint `/api/chat` solo procesa la entrada y devuelve texto generado, pero no invoca `AIAssistant.create_presentation()` o `AIAssistant.create_document()`.
  - Impacto: No se aprovecha la capacidad de creación automática de archivos desde la conversación.

- **F3 - Fallback de presentación demasiado básico**
  - Archivo: `backend/src/reasoning_engine.py`
  - Detalle: El fallback para solicitudes de PowerPoint devuelve solo información descriptiva, no ejecuta una acción concreta.
  - Impacto: El usuario recibe orientación en lugar de resultados prácticos.

- **F4 - Reglas de pentesting demasiado específicas**
  - Archivo: `backend/src/reasoning_engine.py`
  - Detalle: La regla `crear_presentacion_pentesting` solo aplica para temas de pentesting y no para presentaciones generales.
  - Impacto: El sistema no puede manejar presentaciones generales a través del chat.

- **F5 - Generación de PowerPoint básica en el endpoint**
  - Archivo: `backend/server.py`
  - Detalle: El endpoint `/api/create-powerpoint` genera slides con contenido hardcodeado y mínimo, sin investigación real de tema.
  - Impacto: Las presentaciones generadas son superficiales y no se adaptan al tema solicitado.

## 2. Errores y riesgos de operación

- **Bloqueo por sanitización**
  - Archivo: `backend/server.py` y `backend/src/sanitizacion.py`
  - Detalle: Cualquier contenido detectado como inseguro es rechazado antes de procesarse, lo cual es deseable para seguridad, pero puede bloquear prompts legítimos si contienen texto complejo.
  - Impacto: Usuarios pueden ver errores de seguridad al enviar instrucciones válidas.

- **Dependencia de Ollama para razonamiento directo**
  - Archivo: `backend/src/ollama_engine.py`
  - Detalle: El nuevo modo directo depende de que Ollama esté disponible y el modelo `gemma` funcione correctamente.
  - Impacto: Si Ollama no está activo, el modo directo no funcionará y el backend deberá caer al razonamiento básico.

## 3. Cambios implementados recientemente

- Se agregaron credenciales codificadas en Base64 en `backend/server.py`:
  - `Alex:Aritzu` para modo directo.
  - `root:root` para modo mantenimiento.

- Se agregó soporte de autenticación en el frontend `web/index.html` y `web/script.js` para enviar `auth_username` y `auth_password` junto a cada mensaje.

- Se creó la excepción de texto directo cuando se detecta el patrón `todo el mensaje` o `contacto directo`.

- Se implementó un modo de mantenimiento que devuelve un diagnóstico técnico en lugar de una conversación normal.

## 4. Recomendaciones adicionales

- Validar el comportamiento real de los modos directo y mantenimiento mediante pruebas con mensajes que usan las credenciales exactas.
- Revisar las respuestas del endpoint `/api/chat` con el nuevo modo directo para confirmar que no se aplica sanitización ni corrección tipográfica.
- Mejorar la generación de presentaciones en `backend/src/ai_assistant.py` para incluir un análisis de tema más profundo y estructuras de slides dinámicas.

## 5. Archivo de referencia

- `Funcionamiento/INFORME_PRUEBA_FUNCIONAL.md` contiene la documentación original de fallos identificados y propuestas de corrección.
