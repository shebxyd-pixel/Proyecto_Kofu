import os
from typing import Any, Dict, List, Optional, Tuple

from file_processor import FileProcessor

from office_agent import OfficeAgent, OUTPUT_DIR, ARCHIVOS_DIR
from ollama_client import NoModelAvailableError
from reasoning import ChainOfThought, HybridReasoningEngine, ReasoningStep
from sanitization import TypoCorrector
from web_research import WebResearcher


class AIAssistant:
    def __init__(self, use_ollama: bool = True):
        self.typo_corrector = TypoCorrector()
        self.web_researcher = WebResearcher()
        self.office_agent = OfficeAgent()
        self.file_processor = FileProcessor()
        self.reasoning_engine = HybridReasoningEngine(use_ollama=use_ollama)
        self.chain_of_thought = ChainOfThought()

    @staticmethod
    def _permite_internet(modo: str) -> bool:
        return modo == "online"

    def _preparar_entrada(self, user_input: str) -> str:
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

    def research_topic(self, topic: str, modo: str = "online", model: Optional[str] = None, style: str = "académico", target_pages: str = "12 a 15 páginas") -> str:
        if not self._permite_internet(modo):
            return "La investigación web requiere modo 'online' (necesita conexión a internet)."
        
        # Varias consultas engañando a Ollama
        consultas = [topic, f"detalles e información técnica sobre {topic}", f"ejemplos y casos de uso de {topic}"]
        all_results = []
        for q in consultas:
            res = self.web_researcher.search_web(q, num_results=3)
            all_results.extend(res)
        
        # Escribiendo todo en crudo en un .md
        raw_info = self.web_researcher.generate_summary(topic, all_results)
        raw_path = os.path.join(OUTPUT_DIR, f"raw_research_{topic[:20].replace(' ', '_')}.md")
        os.makedirs(os.path.dirname(os.path.abspath(raw_path)), exist_ok=True)
        with open(raw_path, "w", encoding="utf-8") as f:
            f.write(raw_info)
            
        # Ollama genera un documento extenso por capítulos
        if self.reasoning_engine.ollama_engine:
            try:
                # Pedimos un índice
                prompt_indice = f"Genera un índice con 8 capítulos principales para un documento con tono {style} MUY extenso sobre '{topic}'. Devuelve SOLO los nombres de los capítulos, uno por línea."
                indice_texto, modelo_usado = self.reasoning_engine.ollama_engine.razonar(prompt_indice, model=model)
                capitulos = [c.strip("-*1234567890. ") for c in indice_texto.split('\n') if len(c.strip()) > 3][:8]
                
                if not capitulos:
                    capitulos = ["Introducción a " + topic, "Contexto y Antecedentes", "Análisis Principal", "Desarrollo del Tema", "Casos de Estudio", "Impacto Actual", "Perspectivas Futuras", "Conclusiones"]

                documento_extenso = ""
                for cap in capitulos:
                    prompt_capitulo = (
                        f"Escribe de manera sumamente extensa, detallada y con tono {style} el capítulo titulado '{cap}' "
                        f"para un documento sobre '{topic}'.\nEl documento completo medirá unas {target_pages}, por lo que este capítulo DEBE ser extremadamente largo y profundo (equivale a 2 o 3 páginas de contenido denso).\n"
                        f"REGLA CRÍTICA: NO incluyas ninguna introducción ni conclusión general en este capítulo (a menos que el capítulo se llame expresamente Introducción o Conclusión). Enfócate única y exclusivamente en desarrollar el subtema '{cap}'.\n"
                        f"Utiliza esta información de internet como base:\n{raw_info[:3000]}"
                    )
                    texto_cap, _ = self.reasoning_engine.ollama_engine.razonar(prompt_capitulo, model=model)
                    documento_extenso += f"\n\n{texto_cap}\n"

                self.reasoning_engine.last_model_used = modelo_usado
                return documento_extenso.strip()
            except Exception:
                pass
        
        return raw_info

    def get_office_tips(self, software: str) -> List[str]:
        if not self.reasoning_engine.ollama_engine:
            return ["Instala Ollama para obtener consejos personalizados."]
        try:
            prompt = f"Dame 5 consejos rápidos y útiles para usar {software}. Devuélvelos como una lista de viñetas, sin introducciones."
            respuesta, _ = self.reasoning_engine.ollama_engine.razonar(prompt)
            return [line.strip("-* ") for line in respuesta.split("\n") if line.strip()]
        except Exception:
            return [f"Consejo genérico para {software}: guarda tu documento frecuentemente."]

    def create_presentation(self, topic: str, output_path: Optional[str] = None,
                             theme: str = "professional", modo: str = "online",
                             filename: Optional[str] = None,
                             template: Optional[str] = None,
                             model: Optional[str] = None) -> Dict[str, Any]:
        research = self.research_topic(topic, modo, model, style=theme, target_pages="10 a 15 diapositivas")
        safe_name = filename or f"presentacion_{topic[:30].replace(' ', '_')}.pptx"
        out = output_path or os.path.join(OUTPUT_DIR, safe_name)
        os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)

        template_path = None
        schema = None
        if template:
            if not template.endswith(".potx"):
                template += ".potx"
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
                         template: Optional[str] = None,
                         model: Optional[str] = None) -> Dict[str, Any]:
        import random
        if not template:
            word_templates_dir = os.path.join(os.path.dirname(__file__), '..', '..', 'templates', 'word')
            if os.path.exists(word_templates_dir):
                available = [f for f in os.listdir(word_templates_dir) if f.endswith(('.dotx', '.docx')) and not f.startswith('~$')]
                if available:
                    template = random.choice(available)
            if style == "professional":
                style = random.choice(["académico", "creativo", "corporativo", "periodístico", "técnico", "formal", "persuasivo", "analítico"])

        research = self.research_topic(topic, modo, model, style=style, target_pages="12 a 15 hojas")
        safe_name = filename or f"documento_{topic[:30].replace(' ', '_')}.docx"
        out = output_path or os.path.join(OUTPUT_DIR, safe_name)
        os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)

        template_path = None
        schema = None
        if template:
            if not template.endswith(".dotx"):
                template += ".dotx"
            template_path = os.path.join(
                os.path.dirname(__file__), '..', '..', 'templates', 'word', template
            )
            schema = self.office_agent.get_template_schema(template)

        subtitle = "Un análisis exhaustivo"
        if self.reasoning_engine.ollama_engine:
            try:
                sub_prompt = f"Genera un subtítulo corto, elegante y profesional (máximo 6 palabras) para un documento titulado '{topic}'. Devuelve ÚNICAMENTE el subtítulo, sin comillas."
                sub_res, _ = self.reasoning_engine.ollama_engine.razonar(sub_prompt, model=model)
                if sub_res.strip():
                    subtitle = sub_res.strip().strip('"').strip("'")
            except Exception:
                pass

        if schema and "sections" in schema:
            content = [
                {"type": "document_title", "text": topic},
                {"type": "document_subtitle", "text": subtitle}
            ]
            for section in schema["sections"]:
                content.append({"type": "heading", "text": section, "level": 2})
                content.append({"type": "paragraph", "text": research})
        else:
            content = [
                {"type": "document_title", "text": topic},
                {"type": "document_subtitle", "text": subtitle},
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
