from .web_researcher import WebResearcher
from .office_agent import OfficeAgent
from .reasoning_engine import ReasoningEngine, ChainOfThought


class KnowledgeBase:
    def __init__(self):
        self.powerpoint_tips = [
            "Usa la regla de 6x6: máximo 6 líneas por diapositiva y 6 palabras por línea",
            "Aplica temas consistentes para mantener el diseño profesional",
            "Usa transiciones moderadas para no distraer",
            "Las animaciones deben ser funcionales, no decorativas",
            "Incluye notas del orador para información adicional"
        ]
        
        self.word_tips = [
            "Usa estilos predefinidos para títulos y párrafos",
            "Aplica sangría francesa para listas de referencias",
            "Usa la función de navegación para moverse rápidamente",
            "Habilita la revisión de cambios para colaboraciones",
            "Usa plantillas (.dotx) para mantener la coherencia"
        ]
        
        self.pentesting_knowledge = {
            "metodologias": ["OWASP", "PTES", "OSSTMM", "NIST"],
            "herramientas": ["Nmap", "Metasploit", "Wireshark", "Burp Suite", "Hydra", "John the Ripper"],
            "fases": ["Reconocimiento", "Escaneo", "Enumeración", "Explotación", "Post-explotación", "Informes"]
        }

    def get_powerpoint_tips(self):
        return self.powerpoint_tips

    def get_word_tips(self):
        return self.word_tips

    def get_pentesting_info(self):
        return self.pentesting_knowledge


class AIAssistant:
    def __init__(self):
        self.web_researcher = WebResearcher()
        self.office_agent = OfficeAgent()
        self.knowledge_base = KnowledgeBase()
        self.reasoning_engine = ReasoningEngine()
        self.chain_of_thought = ChainOfThought()

    def process_request(self, user_input: str, show_thinking: bool = False):
        self.chain_of_thought.steps = []
        self.chain_of_thought.add_step(f"Analizando solicitud: {user_input}")
        
        response, thinking_steps = self.reasoning_engine.reason(user_input)
        
        if show_thinking:
            thinking_process = "\n".join([str(step) for step in thinking_steps])
            response += f"\n\n--- Proceso de razonamiento ---\n{thinking_process}"
        
        self.chain_of_thought.add_step("Generando respuesta final")
        
        return response, thinking_steps

    def research_topic(self, topic):
        self.chain_of_thought.add_step(f"Iniciando investigación sobre: {topic}")
        results = self.web_researcher.search_web(topic)
        
        if results:
            self.chain_of_thought.add_step("Encontradas fuentes relevantes")
            summary = self.web_researcher.generate_summary(topic, results)
            self.chain_of_thought.add_step("Generando resumen estructurado")
            return summary
        else:
            self.chain_of_thought.add_step("No se encontraron fuentes")
        return "No se encontró información sobre este tema."

    def get_office_tips(self, software):
        if software.lower() == "powerpoint":
            return self.knowledge_base.get_powerpoint_tips()
        elif software.lower() == "word":
            return self.knowledge_base.get_word_tips()
        return []

    def get_pentesting_knowledge(self):
        return self.knowledge_base.get_pentesting_info()

    def create_presentation(self, topic, output_path, theme="professional"):
        self.chain_of_thought.steps = []
        self.chain_of_thought.add_step(f"Creando presentación sobre: {topic}")
        self.chain_of_thought.add_step(f"Usando tema: {theme}")
        
        research = self.research_topic(topic)
        
        self.chain_of_thought.add_step("Generando estructura de slides")
        slides_data = [
            {
                "title": f"Presentación sobre: {topic}",
                "layout": 0,
                "subtitle": "Generado automáticamente",
                "background": True
            },
            {
                "title": "Introducción",
                "layout": 1,
                "text": research[:300] + "...",
                "background": True
            },
            {
                "title": "Conclusiones",
                "layout": 5,
                "text": "Para más información, consulte las fuentes originales.",
                "background": True
            }
        ]
        
        self.chain_of_thought.add_step("Aplicando formato y guardando")
        result = self.office_agent.create_powerpoint(
            output_path,
            slides_data,
            theme=theme
        )
        
        self.chain_of_thought.add_step("Presentación creada exitosamente")
        return result

    def create_document(self, topic, output_path, style="professional"):
        self.chain_of_thought.steps = []
        self.chain_of_thought.add_step(f"Creando documento sobre: {topic}")
        self.chain_of_thought.add_step(f"Usando estilo: {style}")
        
        research = self.research_topic(topic)
        
        self.chain_of_thought.add_step("Generando estructura del documento")
        content_data = [
            {
                "type": "heading",
                "text": f"Documento sobre: {topic}",
                "level": 1
            },
            {
                "type": "paragraph",
                "text": research
            },
            {
                "type": "heading",
                "text": "Conocimiento adicional",
                "level": 2
            },
            {
                "type": "paragraph",
                "text": "Este documento fue generado automáticamente con información de fuentes públicas."
            }
        ]
        
        self.chain_of_thought.add_step("Aplicando formato y guardando")
        result = self.office_agent.create_word_document(
            output_path,
            content_data,
            style=style
        )
        
        self.chain_of_thought.add_step("Documento creado exitosamente")
        return result

    def get_thinking_chain(self):
        return self.chain_of_thought.explain()
