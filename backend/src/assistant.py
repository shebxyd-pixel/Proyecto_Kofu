import os
from typing import Any, Dict, List, Optional, Tuple

from file_processor import FileProcessor
from knowledge_base import KnowledgeBase
from office_agent import OfficeAgent, OUTPUT_DIR, ARCHIVOS_DIR
from ollama_client import NoModelAvailableError
from reasoning import ChainOfThought, HybridReasoningEngine, ReasoningStep
from sanitization import SanitizadorEntrada, TypoCorrector
from web_research import WebResearcher


class AIAssistant:
    def __init__(self, use_ollama: bool = True):
        self.sanitizador = SanitizadorEntrada()
        self.typo_corrector = TypoCorrector()
        self.web_researcher = WebResearcher()
        self.office_agent = OfficeAgent()
        self.file_processor = FileProcessor()
        self.knowledge_base = KnowledgeBase()
        self.reasoning_engine = HybridReasoningEngine(use_ollama=use_ollama)
        self.chain_of_thought = ChainOfThought()

    @staticmethod
    def _permite_internet(modo: str) -> bool:
        return modo == "online"

    def _preparar_entrada(self, user_input: str) -> str:
        if not self.sanitizador.es_entrada_segura(user_input):
            user_input = self.sanitizador.limpiar_texto(user_input)
        return self.typo_corrector.correct_text(user_input)

    def process_request(self, user_input: str, modo: str = "local",
                         model: Optional[str] = None) -> Tuple[str, List[ReasoningStep]]:
        entrada = self._preparar_entrada(user_input)
        self.chain_of_thought.steps = []
        self.chain_of_thought.add_step(f"Analizando solicitud ({modo}, modelo={model or 'auto'}): {entrada}")
        response, steps = self.reasoning_engine.reason(
            entrada, allow_external=self._permite_internet(modo), model=model
        )
        self.chain_of_thought.add_step("Generando respuesta final")
        return response, steps

    def direct_reason(self, user_input: str, modo: str = "local",
                       model: Optional[str] = None) -> Tuple[str, List[ReasoningStep]]:
        entrada = self._preparar_entrada(user_input)
        self.chain_of_thought.steps = []
        self.chain_of_thought.add_step(f"Acceso directo ({modo}, modelo={model or 'auto'}): {entrada}")
        response, steps = self.reasoning_engine.direct_reason(
            entrada, allow_external=self._permite_internet(modo), model=model
        )
        self.chain_of_thought.add_step("Respuesta directa generada")
        return response, steps

    def research_topic(self, topic: str, modo: str = "online") -> str:
        if not self._permite_internet(modo):
            return "La investigación web requiere modo 'online' (necesita conexión a internet)."
        results = self.web_researcher.search_web(topic)
        return self.web_researcher.generate_summary(topic, results)

    def get_office_tips(self, software: str) -> List[str]:
        return {"powerpoint": self.knowledge_base.POWERPOINT_TIPS,
                "word": self.knowledge_base.WORD_TIPS}.get(software.lower(), [])

    def create_presentation(self, topic: str, output_path: Optional[str] = None,
                             theme: str = "professional", modo: str = "online",
                             filename: Optional[str] = None,
                             template: Optional[str] = None) -> Dict[str, Any]:
        research = self.research_topic(topic, modo)
        safe_name = filename or f"presentacion_{topic[:30].replace(' ', '_')}.pptx"
        out = output_path or os.path.join(OUTPUT_DIR, safe_name)
        os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)

        template_path = None
        schema = None
        if template:
            template_path = os.path.join(
                os.path.dirname(__file__), '..', '..', 'templates', 'powerpoint', template
            )
            schema = self.office_agent.get_template_schema(template)

        if schema and "slides" in schema:
            slides = []
            for s in schema["slides"]:
                slide = dict(s)
                slide["title"] = slide.get("title", "").replace("{topic}", topic)
                slide["text"] = slide.get("text", "").replace("{intro}", research[:200]).replace(
                    "{body}", research[:400]).replace("{conclusion}", research[-200:])
                slides.append(slide)
        else:
            slides = [
                {"title": f"Presentación sobre: {topic}", "layout": 0, "subtitle": "Generado automáticamente", "background": True},
                {"title": "Introducción", "layout": 1, "text": research[:300] + "...", "background": True},
                {"title": "Conclusiones", "layout": 5, "text": "Para más información, consulte las fuentes originales.", "background": True},
            ]

        result = self.office_agent.create_powerpoint(out, slides, template_path=template_path, theme=theme)
        result["filename"] = os.path.basename(out)
        return result

    def create_document(self, topic: str, output_path: Optional[str] = None,
                         style: str = "professional", modo: str = "online",
                         filename: Optional[str] = None,
                         template: Optional[str] = None) -> Dict[str, Any]:
        research = self.research_topic(topic, modo)
        safe_name = filename or f"documento_{topic[:30].replace(' ', '_')}.docx"
        out = output_path or os.path.join(OUTPUT_DIR, safe_name)
        os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)

        template_path = None
        schema = None
        if template:
            template_path = os.path.join(
                os.path.dirname(__file__), '..', '..', 'templates', 'word', template
            )
            schema = self.office_agent.get_template_schema(template)

        if schema and "sections" in schema:
            content = [{"type": "heading", "text": topic, "level": 1}]
            for section in schema["sections"]:
                content.append({"type": "heading", "text": section, "level": 2})
                content.append({"type": "paragraph", "text": research[:300]})
        else:
            content = [
                {"type": "heading", "text": f"Documento sobre: {topic}", "level": 1},
                {"type": "paragraph", "text": research},
                {"type": "heading", "text": "Conocimiento adicional", "level": 2},
                {"type": "paragraph", "text": "Este documento fue generado automáticamente con información de fuentes públicas."},
            ]

        result = self.office_agent.create_word_document(out, content, template_path=template_path, style=style)
        result["filename"] = os.path.basename(out)
        return result

    def digest_file(self, file_path: str, output_path: Optional[str] = None,
                     instrucciones: Optional[str] = None, model: Optional[str] = None) -> Dict[str, Any]:
        extraido = self.file_processor.process_local_file(file_path)
        if not extraido.get("success"):
            return extraido

        texto_preparado = self._preparar_entrada(extraido["text_content"])

        prompt = instrucciones or "Resume y estructura el siguiente contenido de forma clara y organizada:"
        digestion = texto_preparado
        if self.reasoning_engine.ollama_engine:
            try:
                digestion, modelo_usado = self.reasoning_engine.ollama_engine.razonar(
                    f"{prompt}\n\n{texto_preparado}", model=model
                )
                self.reasoning_engine.last_model_used = modelo_usado
            except NoModelAvailableError:
                raise
            except RuntimeError:
                pass

        resultado = {"success": True, "filename": extraido["filename"], "digestion": digestion}

        if output_path:
            os.makedirs(os.path.dirname(os.path.abspath(output_path)) or ".", exist_ok=True)
            with open(output_path, "w", encoding="utf-8") as f:
                f.write(digestion)
            resultado["output_path"] = os.path.abspath(output_path)

        return resultado
