# Instrucciones de Uso del Agente de IA Completo

## Paso 1: Iniciar el Servidor

Abre una terminal y ejecuta:

```bash
py server.py
```

El servidor se iniciará en `http://localhost:5000`

## Paso 2: Abrir el Chatbot

Abre el archivo `index.html` en tu navegador.

## Paso 3: Usar el Chatbot

Ahora el chatbot está conectado al servidor real y puede:

- Responder a preguntas sobre pentesting con información real
- Mostrar su proceso de razonamiento paso a paso
- Conectarse al motor de razonamiento completo

## API Endpoints Disponibles

### POST /api/chat
Envía un mensaje y recibe una respuesta con razonamiento.

**Ejemplo:**
```json
{
  "message": "qué es el pentesting",
  "show_thinking": true
}
```

### POST /api/research
Investiga un tema en internet.

**Ejemplo:**
```json
{
  "topic": "inteligencia artificial"
}
```

### POST /api/create-powerpoint
Crea una presentación de PowerPoint.

**Ejemplo:**
```json
{
  "topic": "seguridad informática",
  "theme": "modern",
  "filename": "presentacion.pptx"
}
```

### POST /api/create-document
Crea un documento de Word.

**Ejemplo:**
```json
{
  "topic": "pentesting",
  "style": "professional",
  "filename": "documento.docx"
}
```

### GET /api/office-tips
Obtén consejos de PowerPoint o Word.

**Parámetro:** `software=powerpoint` o `software=word`

## Temas Disponibles

### PowerPoint:
- `professional` - Azul corporativo
- `modern` - Verde tecnológico
- `vibrant` - Naranja dinámico

### Word:
- `professional` - Estilo clásico
- `modern` - Estilo actual

## Razonamiento Completo

El agente muestra su proceso de pensamiento:
1. Analiza la entrada del usuario
2. Identifica el tema
3. Aplica reglas de conocimiento
4. Deduce una conclusión
5. Genera la respuesta final
