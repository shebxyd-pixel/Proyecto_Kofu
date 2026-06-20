import sys
import os
import io
import re
import base64

try:
    from flask import Flask, request, jsonify
    from flask_cors import CORS
    from dotenv import load_dotenv
    import requests
    from bs4 import BeautifulSoup
    import lxml
    from docx import Document
    from pptx import Presentation
    import win32com.client
except ImportError as e:
    print("=" * 50)
    print("ERROR: Faltan dependencias necesarias")
    print("=" * 50)
    print()
    print(f"Modulo faltante: {e.name}")
    print()
    print("Por favor, instala todas las dependencias ejecutando:")
    print("  pip install -r requirements.txt")
    print()
    print("=" * 50)
    sys.exit(1)

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

try:
    from src import AIAssistant, TypoCorrector, ExternalAIAPI, OfficeAgent, SanitizadorEntrada, FileProcessor
    from src.web_researcher import WebResearcher
except ImportError as e:
    print("=" * 50)
    print("ERROR: Faltan dependencias del módulo src")
    print("=" * 50)
    print()
    print(f"Error: {e}")
    print()
    print("Por favor, instala todas las dependencias ejecutando:")
    print("  pip install -r requirements.txt")
    print()
    print("=" * 50)
    sys.exit(1)

app = Flask(__name__)
CORS(app)

BASE_DIR = os.path.join(os.path.dirname(__file__), '..')

# Cargar variables de entorno
load_dotenv(os.path.join(BASE_DIR, ".env"))

use_ollama = os.getenv("USE_LOCAL_LLM", os.getenv("USE_OLLAMA", "true")).lower() == "true"
ollama_model = os.getenv("OLLAMA_MODEL", os.getenv("LLM_MODEL_NAME", "gemma4"))
assistant = AIAssistant(use_ollama=use_ollama, ollama_model=ollama_model)
typo_corrector = TypoCorrector()
external_ai = ExternalAIAPI()
office_agent = OfficeAgent()
web_researcher = WebResearcher()
sanitizador = SanitizadorEntrada()
file_processor = FileProcessor()

DIRECT_ACCESS_TOKEN = base64.b64encode(b"Alex:Aritzu").decode('utf-8')
MAINTENANCE_ACCESS_TOKEN = base64.b64encode(b"root:root").decode('utf-8')


def _encode_auth(username: str, password: str) -> str:
    return base64.b64encode(f"{username}:{password}".encode('utf-8')).decode('utf-8')


def _get_auth_mode(message: str) -> str:
    username_match = re.search(r'USERNAME\s*[:=]\s*([A-Za-z0-9_@.-]+)', message, re.IGNORECASE)
    password_match = re.search(r'PASSWORD\s*[:=]\s*([A-Za-z0-9_@.-]+)', message, re.IGNORECASE)
    if not username_match or not password_match:
        return ''

    username = username_match.group(1).strip()
    password = password_match.group(1).strip()
    encoded = _encode_auth(username, password)
    if encoded == DIRECT_ACCESS_TOKEN:
        return 'direct'
    if encoded == MAINTENANCE_ACCESS_TOKEN:
        return 'maintenance'
    return ''


def _remove_auth_lines(message: str) -> str:
    message = re.sub(r'USERNAME\s*[:=]\s*[A-Za-z0-9_@.-]+', '', message, flags=re.IGNORECASE)
    message = re.sub(r'PASSWORD\s*[:=]\s*[A-Za-z0-9_@.-]+', '', message, flags=re.IGNORECASE)
    return re.sub(r'\s{2,}', ' ', message).strip()


def _is_direct_exception(message: str) -> bool:
    if not isinstance(message, str):
        return False
    direct_triggers = [r'\btodo el mensaje\b', r'\bcontacto directo\b']
    return any(re.search(trigger, message, re.IGNORECASE) for trigger in direct_triggers)


def _maintenance_debug_response(message: str) -> str:
    availability = False
    model_name = 'desconocido'
    if getattr(assistant.reasoning_engine, 'ollama_engine', None):
        availability = assistant.reasoning_engine.ollama_engine.available
        model_name = assistant.reasoning_engine.ollama_engine.model_name
    status_lines = [
        'Modo mantenimiento raíz activado: solo diagnóstico técnico.',
        f'Ollama disponible: {availability}',
        f'Modelo Ollama: {model_name}',
        f'Sanitizador activo: {sanitizador is not None}',
        f'Typo corrector activo: {typo_corrector is not None}',
        f"Mensaje recibido: {message[:400] or '<vacío>'}",
        'Reglas del motor de razonamiento cargadas: ' + ', '.join([rule.name for rule in assistant.reasoning_engine.rules])
    ]
    if any(term in message.lower() for term in ['error', 'fallo', 'problema', 'excepción', 'exception', 'bug']):
        status_lines.append('Solicitud de reporte de error detectada.')
    else:
        status_lines.append('Respuesta de mantenimiento: se devolverá solo estado y problemas detectados, no conversación normal.')

    return '\n'.join(status_lines)

if not os.path.exists(os.path.join(BASE_DIR, "output")):
    os.makedirs(os.path.join(BASE_DIR, "output"))
if not os.path.exists(os.path.join(BASE_DIR, "temp_uploads")):
    os.makedirs(os.path.join(BASE_DIR, "temp_uploads"))


@app.route('/')
def index():
    return "Servidor de IA funcionando! Usa la API /api/chat para interactuar."


@app.route('/api/templates', methods=['GET'])
def get_templates():
    try:
        ppt_templates = []
        word_templates = []

        ppt_path = os.path.join(BASE_DIR, "templates/powerpoint")
        if os.path.exists(ppt_path):
            ppt_templates = [f for f in os.listdir(ppt_path) if f.endswith(('.pptx', '.potx', '.pptm'))]

        word_path = os.path.join(BASE_DIR, "templates/word")
        if os.path.exists(word_path):
            word_templates = [f for f in os.listdir(word_path) if f.endswith(('.docx', '.dotx', '.docm'))]

        return jsonify({
            'powerpoint': ppt_templates,
            'word': word_templates
        })

    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/correct-typo', methods=['POST'])
def correct_typo():
    try:
        data = request.json
        text = data.get('text', '')
        
        if not text:
            return jsonify({'error': 'No se proporcionó texto'}), 400
        
        corrected = typo_corrector.correct_text(text)
        
        return jsonify({
            'original': text,
            'corrected': corrected
        })
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/external-ai', methods=['POST'])
def query_external_ai():
    try:
        data = request.json
        query = data.get('query', '')
        use_google = data.get('use_google', True)
        use_openai = data.get('use_openai', True)
        
        if not query:
            return jsonify({'error': 'No se proporcionó consulta'}), 400
        
        results = external_ai.get_response(query, use_google, use_openai)
        
        return jsonify({
            'query': query,
            'results': results
        })
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/chat', methods=['POST'])
def chat():
    try:
        data = request.json
        message = data.get('message', '')
        show_thinking = data.get('show_thinking', False)

        auth_mode = _get_auth_mode(message)
        direct_access = auth_mode == 'direct' or _is_direct_exception(message)

        if auth_mode == 'maintenance':
            return jsonify({
                'maintenance': True,
                'response': _maintenance_debug_response(message)
            })

        clean_message = _remove_auth_lines(message)

        if not clean_message:
            return jsonify({'error': 'No se proporcionó mensaje válido después de eliminar las credenciales. Añade una instrucción junto con USERNAME/Password.'}), 400

        if direct_access:
            # try:
            #     response, thinking_steps = assistant.direct_reason(clean_message)
            # except Exception as e:
            #     return jsonify({
            #         'error': f'Modo directo falló: {str(e)}. Verifica que Ollama esté disponible y corriendo.'
            #     }), 500

            # thinking_process = [
            #     {"step": step.step_num, "thought": step.thought, "evidence": step.evidence}
            #     for step in thinking_steps
            # ]
            return jsonify({
                'original_message': message,
                'mode': 'direct_raw',
                'response': clean_message,
                'thinking_process': [],
                'thinking_chain': ''
            })

        # SANITIZAR ENTRADA
        # if not sanitizador.es_entrada_segura(message):
        #     return jsonify({"error": "❌ Entrada bloqueada por seguridad: contiene código o contenido no permitido"}), 400
        # message = sanitizador.limpiar_texto(message)

        # corrected_message = typo_corrector.correct_text(message)
        # response, thinking_steps = assistant.process_request(corrected_message, show_thinking=show_thinking)
        response = clean_message
        thinking_steps = []

        thinking_process = [
            {"step": step.step_num, "thought": step.thought, "evidence": step.evidence}
            for step in thinking_steps
        ]

        return jsonify({
            'original_message': message,
            'corrected_message': corrected_message,
            'response': response,
            'thinking_process': thinking_process,
            'thinking_chain': assistant.get_thinking_chain()
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/search', methods=['POST'])
def search():
    try:
        data = request.json
        query = data.get('query', '')
        num_results = data.get('num_results', 5)
        engine = data.get('engine', 'duckduckgo')

        # SANITIZAR ENTRADA
        if not sanitizador.es_entrada_segura(query):
            return jsonify({"error": "❌ Entrada bloqueada por seguridad: contiene código o contenido no permitido"}), 400
        query = sanitizador.limpiar_texto(query)

        if not query:
            return jsonify({'error': 'No se proporcionó consulta de búsqueda válida'}), 400

        results = web_researcher.search_web(query, num_results=num_results, engine=engine)

        return jsonify({
            'query': query,
            'results': results,
            'total': len(results)
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/research', methods=['POST'])
def research():
    try:
        data = request.json
        topic = data.get('topic', '')

        # SANITIZAR ENTRADA
        if not sanitizador.es_entrada_segura(topic):
            return jsonify({"error": "❌ Entrada bloqueada por seguridad: contiene código o contenido no permitido"}), 400
        topic = sanitizador.limpiar_texto(topic)

        if not topic:
            return jsonify({'error': 'No se proporcionó tema válido'}), 400

        summary = assistant.research_topic(topic)

        return jsonify({
            'topic': topic,
            'summary': summary
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/create-powerpoint', methods=['POST'])
def create_powerpoint():
    try:
        data = request.json
        topic = data.get('topic', 'Presentacion')
        theme = data.get('theme', 'professional')
        filename = data.get('filename', 'presentacion_kofu.pptx')
        template = data.get('template', None)

        # SANITIZAR ENTRADAS
        if not sanitizador.es_entrada_segura(topic):
            return jsonify({"error": "❌ Entrada bloqueada por seguridad: contiene código o contenido no permitido"}), 400
        topic = sanitizador.limpiar_texto(topic)
        filename = sanitizador.sanitizar_nombre_archivo(filename)

        template_path = None
        if template:
            template = sanitizador.sanitizar_nombre_archivo(template)
            template_path = os.path.join(BASE_DIR, 'templates/powerpoint', template)

        output_path = os.path.join(BASE_DIR, 'output', filename)

        slides_data = [
            {"title": topic, "layout": 0, "subtitle": "Generado por Kofu", "background": True},
            {"title": "Contenido", "layout": 1, "text": f"Presentacion sobre {topic} creada automaticamente.", "background": True}
        ]

        result = office_agent.create_powerpoint(
            output_path,
            slides_data,
            template_path=template_path,
            theme=theme
        )

        return jsonify({
            'success': True,
            'file_path': result,
            'topic': topic,
            'theme': theme,
            'template': template
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/create-document', methods=['POST'])
def create_document():
    try:
        data = request.json
        topic = data.get('topic', 'Documento')
        style = data.get('style', 'professional')
        filename = data.get('filename', 'documento_kofu.docx')
        template = data.get('template', None)

        # SANITIZAR ENTRADAS
        if not sanitizador.es_entrada_segura(topic):
            return jsonify({"error": "❌ Entrada bloqueada por seguridad: contiene código o contenido no permitido"}), 400
        topic = sanitizador.limpiar_texto(topic)
        filename = sanitizador.sanitizar_nombre_archivo(filename)

        template_path = None
        if template:
            template = sanitizador.sanitizar_nombre_archivo(template)
            template_path = os.path.join(BASE_DIR, 'templates/word', template)

        output_path = os.path.join(BASE_DIR, 'output', filename)

        content_data = [
            {"type": "heading", "text": topic, "level": 1},
            {"type": "paragraph", "text": f"Documento sobre {topic} generado automaticamente por Kofu."}
        ]

        result = office_agent.create_word_document(
            output_path,
            content_data,
            template_path=template_path,
            style=style
        )

        return jsonify({
            'success': True,
            'file_path': result,
            'topic': topic,
            'style': style,
            'template': template
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/office-tips', methods=['GET'])
def office_tips():
    try:
        software = request.args.get('software', 'powerpoint')
        tips = assistant.get_office_tips(software)
        
        return jsonify({
            'software': software,
            'tips': tips
        })
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/upload', methods=['POST'])
def upload_file():
    try:
        if 'file' not in request.files:
            return jsonify({'error': 'No file part'}), 400
        
        file = request.files['file']
        if file.filename == '':
            return jsonify({'error': 'No selected file'}), 400
        
        if not file_processor.is_supported_file(file.filename):
            return jsonify({'error': 'Unsupported file type'}), 400
        
        # Sanitize filename
        filename = sanitizador.sanitizar_nombre_archivo(file.filename)
        
        # Read file stream
        file_stream = io.BytesIO(file.read())
        
        # Process the file
        result = file_processor.process_file(file_stream, filename)
        
        return jsonify(result)
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/create-document-from-file', methods=['POST'])
def create_document_from_file():
    try:
        if 'file' not in request.files:
            return jsonify({'error': 'No file part'}), 400
        
        file = request.files['file']
        style = request.form.get('style', 'professional')
        filename = request.form.get('filename', 'documento_kofu.docx')
        template = request.form.get('template', None)
        
        if file.filename == '':
            return jsonify({'error': 'No selected file'}), 400
        
        if not file_processor.is_supported_file(file.filename):
            return jsonify({'error': 'Unsupported file type'}), 400
        
        # Sanitize inputs
        filename = sanitizador.sanitizar_nombre_archivo(filename)
        
        # Process the file to get content
        file_stream = io.BytesIO(file.read())
        process_result = file_processor.process_file(file_stream, file.filename)
        
        if not process_result['success']:
            return jsonify({'error': process_result['error']}), 400
        
        # Create document from processed content
        content = process_result['text_content']
        output_path = os.path.join(BASE_DIR, 'output', filename)
        
        template_path = None
        if template:
            template = sanitizador.sanitizar_nombre_archivo(template)
            template_path = os.path.join(BASE_DIR, 'templates/word', template)
        
        # Build content data
        content_data = [
            {"type": "heading", "text": f"Documento basado en {process_result['filename']}", "level": 1},
            {"type": "paragraph", "text": content}
        ]
        
        result = office_agent.create_word_document(
            output_path,
            content_data,
            template_path=template_path,
            style=style
        )
        
        return jsonify({
            'success': True,
            'file_path': result,
            'source_file': process_result['filename'],
            'style': style
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/create-powerpoint-from-file', methods=['POST'])
def create_powerpoint_from_file():
    try:
        if 'file' not in request.files:
            return jsonify({'error': 'No file part'}), 400
        
        file = request.files['file']
        theme = request.form.get('theme', 'professional')
        filename = request.form.get('filename', 'presentacion_kofu.pptx')
        template = request.form.get('template', None)
        
        if file.filename == '':
            return jsonify({'error': 'No selected file'}), 400
        
        if not file_processor.is_supported_file(file.filename):
            return jsonify({'error': 'Unsupported file type'}), 400
        
        # Sanitize inputs
        filename = sanitizador.sanitizar_nombre_archivo(filename)
        
        # Process the file to get content
        file_stream = io.BytesIO(file.read())
        process_result = file_processor.process_file(file_stream, file.filename)
        
        if not process_result['success']:
            return jsonify({'error': process_result['error']}), 400
        
        # Create presentation from processed content
        content = process_result['text_content']
        output_path = os.path.join(BASE_DIR, 'output', filename)
        
        template_path = None
        if template:
            template = sanitizador.sanitizar_nombre_archivo(template)
            template_path = os.path.join(BASE_DIR, 'templates/powerpoint', template)
        
        # Build slides data
        slides_data = [
            {"title": f"Presentación basada en {process_result['filename']}", "layout": 0, "subtitle": "Generado por Kofu", "background": True},
            {"title": "Contenido", "layout": 1, "text": content[:500] + "..." if len(content) > 500 else content, "background": True}
        ]
        
        result = office_agent.create_powerpoint(
            output_path,
            slides_data,
            template_path=template_path,
            theme=theme
        )
        
        return jsonify({
            'success': True,
            'file_path': result,
            'source_file': process_result['filename'],
            'theme': theme
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500


if __name__ == '__main__':
    print("========================================")
    print("   Kofu v0.7 (Beta) - Servidor de IA")
    print("========================================")
    print("Servidor iniciado en http://localhost:5000")
    print("Abre web/index.html en tu navegador para usar el chatbot")
    try:
        from waitress import serve
        print("Usando servidor de producción (Waitress)")
        serve(app, host='0.0.0.0', port=5000)
    except ImportError:
        print("Waitress no disponible, usando servidor de desarrollo de Flask")
        app.run(host='0.0.0.0', port=5000, debug=False)
