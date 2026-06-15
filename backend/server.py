import sys
import os

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
    from src import AIAssistant, TypoCorrector, ExternalAIAPI, OfficeAgent
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

assistant = AIAssistant()
typo_corrector = TypoCorrector()
external_ai = ExternalAIAPI()
office_agent = OfficeAgent()
web_researcher = WebResearcher()

BASE_DIR = os.path.join(os.path.dirname(__file__), '..')

if not os.path.exists(os.path.join(BASE_DIR, "output")):
    os.makedirs(os.path.join(BASE_DIR, "output"))


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
        
        if not message:
            return jsonify({'error': 'No se proporcionó mensaje'}), 400
        
        corrected_message = typo_corrector.correct_text(message)
        response, thinking_steps = assistant.process_request(corrected_message, show_thinking=show_thinking)
        
        thinking_process = [
            {'step': step.step_num, 'thought': step.thought, 'evidence': step.evidence}
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
        
        if not query:
            return jsonify({'error': 'No se proporcionó consulta de búsqueda'}), 400
        
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
        
        if not topic:
            return jsonify({'error': 'No se proporcionó tema'}), 400
        
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
        filename = data.get('filename', 'presentacion.pptx')
        template = data.get('template', None)
        
        template_path = None
        if template:
            template_path = os.path.join(BASE_DIR, 'templates/powerpoint', template)
        
        output_path = os.path.join(BASE_DIR, 'output', filename)
        
        slides_data = [
            {"title": topic, "layout": 0, "subtitle": "Generado por LarIA", "background": True},
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
        filename = data.get('filename', 'documento.docx')
        template = data.get('template', None)
        
        template_path = None
        if template:
            template_path = os.path.join(BASE_DIR, 'templates/word', template)
        
        output_path = os.path.join(BASE_DIR, 'output', filename)
        
        content_data = [
            {"type": "heading", "text": topic, "level": 1},
            {"type": "paragraph", "text": f"Documento sobre {topic} generado automaticamente por LarIA."}
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


if __name__ == '__main__':
    print("========================================")
    print("   LarI.A v0.4 (Beta) - Servidor de IA")
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
