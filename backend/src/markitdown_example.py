from markitdown import MarkItDown
import os

def extraer_contenido_documento(ruta_archivo: str) -> str:
    """
    Lee un archivo local (PDF, DOCX, XLSX, PPTX, etc.) y devuelve
    su contenido estructurado en formato Markdown.
    """
    if not os.path.exists(ruta_archivo):
        return f"Error: El archivo en '{ruta_archivo}' no existe."
    
    try:
        # Inicializamos MarkItDown
        md = MarkItDown()
        
        # Convertimos el archivo de forma automática
        resultado = md.convert(ruta_archivo)
        
        # Devolvemos el texto en Markdown
        return resultado.text_content
        
    except Exception as e:
        return f"Error al procesar el archivo: {str(e)}"

# --- Ejemplo de integración en el flujo de tu agente ---
# 1. El usuario dice: "Analiza el reporte mensual.pdf"
ruta_del_usuario = "reporte_mensual.pdf"

# 2. Tu agente ejecuta la herramienta
contenido_markdown = extraer_contenido_documento(ruta_del_usuario)

# 3. Formateas el prompt para tu LLM local (ej. Ollama)
prompt_para_llm = f"""
El usuario quiere que analices el siguiente documento. 
Aquí tienes el contenido del archivo convertido a Markdown:

---
{contenido_markdown}
---

Por favor, responde a la solicitud del usuario basándote en el texto anterior.
"""

# 4. Envías 'prompt_para_llm' a tu modelo local...