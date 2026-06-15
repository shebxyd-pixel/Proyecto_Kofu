import re
from typing import List, Dict, Any, Tuple


class KnowledgeRule:
    def __init__(self, name: str, conditions: List[str], conclusion: str, explanation: str):
        self.name = name
        self.conditions = conditions
        self.conclusion = conclusion
        self.explanation = explanation


class ReasoningStep:
    def __init__(self, step_num: int, thought: str, evidence: str = ""):
        self.step_num = step_num
        self.thought = thought
        self.evidence = evidence

    def __str__(self):
        return f"Paso {self.step_num}: {self.thought} {f'({self.evidence})' if self.evidence else ''}"


class ReasoningEngine:
    def __init__(self):
        self.rules: List[KnowledgeRule] = self._load_default_rules()
        self.facts: List[str] = []
        self.thinking_steps: List[ReasoningStep] = []

    def _load_default_rules(self) -> List[KnowledgeRule]:
        return [
            KnowledgeRule(
                "crear_documento_pentesting",
                ["necesita informacion sobre pentesting", "quiere aprender sobre seguridad", "necesita un documento de seguridad"],
                "crear_documento_pentesting",
                "El usuario necesita información estructurada sobre pentesting"
            ),
            KnowledgeRule(
                "crear_presentacion_pentesting",
                ["necesita una presentacion de pentesting", "quiere explicar pentesting a otros", "presentacion de seguridad"],
                "crear_presentacion_pentesting",
                "El usuario necesita una presentación visual sobre pentesting"
            ),
            KnowledgeRule(
                "investigar_tema",
                ["necesita informacion sobre", "quiere saber sobre", "investiga", "busca"],
                "investigar_tema",
                "El usuario requiere investigación web sobre un tema específico"
            ),
            KnowledgeRule(
                "dar_consejos_office",
                ["consejos de", "tips de", "trucos de", "como usar"],
                "dar_consejos_office",
                "El usuario necesita consejos sobre herramientas de Office"
            ),
            KnowledgeRule(
                "explicar_pentesting",
                ["que es", "explica", "que son"],
                "explicar_pentesting",
                "El usuario necesita una explicación sobre pentesting"
            )
        ]

    def add_fact(self, fact: str):
        self.facts.append(fact.lower())

    def clear_facts(self):
        self.facts = []
        self.thinking_steps = []

    def _extract_topic(self, text: str) -> str:
        patterns = [
            r"sobre\s+(.+?)(?:\?|$|,|\.)",
            r"de\s+(.+?)(?:\?|$|,|\.)",
            r"acerca\s+de\s+(.+?)(?:\?|$|,|\.)"
        ]
        for pattern in patterns:
            match = re.search(pattern, text.lower())
            if match:
                return match.group(1).strip()
        return ""

    def reason(self, user_input: str) -> Tuple[str, List[ReasoningStep]]:
        self.thinking_steps = []
        self.clear_facts()
        self.add_fact(user_input)

        self.thinking_steps.append(ReasoningStep(1, "Analizando entrada del usuario", user_input))

        topic = self._extract_topic(user_input)
        if topic:
            self.thinking_steps.append(ReasoningStep(2, f"Tema identificado: {topic}"))

        matched_rule = None
        for rule in self.rules:
            matches = sum(1 for cond in rule.conditions if cond in user_input.lower())
            if matches > 0:
                matched_rule = rule
                self.thinking_steps.append(ReasoningStep(3, f"Regla aplicada: {rule.name}", f"Coincide con: {', '.join(rule.conditions[:matches])}"))
                break

        if not matched_rule:
            self.thinking_steps.append(ReasoningStep(3, "Ninguna regla específica aplicada", "Usando razonamiento general"))
            return self._generate_fallback_response(user_input), self.thinking_steps

        self.thinking_steps.append(ReasoningStep(4, f"Conclusión derivada", matched_rule.conclusion))

        response = self._generate_response(matched_rule, topic)
        self.thinking_steps.append(ReasoningStep(5, "Generando respuesta final"))

        return response, self.thinking_steps

    def _generate_fallback_response(self, user_input: str) -> str:
        lower_input = user_input.lower()
        
        if any(word in lower_input for word in ['pentesting', 'seguridad', 'hacker', 'prueba de penetración']):
            return 'Sobre pentesting:\n• Definición: Evaluación de seguridad simulando ataques\n• Metodologías: OWASP, PTES, OSSTMM, NIST\n• Fases: Reconocimiento → Escaneo → Enumeración → Explotación → Post-explotación → Informes\n• Herramientas: Nmap, Metasploit, Wireshark, Burp Suite\n\n¿Quieres que cree un documento o presentación?';
        
        if any(word in lower_input for word in ['powerpoint', 'presentación', 'pptx', 'ppt']):
            return 'Para presentaciones puedo:\n• Usar temas: professional (azul), modern (verde), vibrant (naranja)\n• Aplicar colores coordinados\n• Formatear títulos y texto automáticamente\n• Incluir contenido desde investigación web\n\n¿Sobre qué tema te gustaría?';
        
        if any(word in lower_input for word in ['word', 'documento', 'docx', 'doc']):
            return 'Para documentos puedo:\n• Usar estilos: professional o modern\n• Crear títulos, párrafos y tablas\n• Aplicar formatos avanzados\n• Incluir contenido desde investigación web\n\n¿Sobre qué tema te gustaría?';
        
        if any(word in lower_input for word in ['investiga', 'busca', 'qué es', 'explica']):
            topic = self._extract_topic(user_input)
            if topic:
                return f'¡Claro! Investigaré sobre "{topic}" y te comparto un resumen. Usa el servidor Python para ver la investigación completa.';
        
        return 'Puedo ayudarte con:\n• Información sobre pentesting\n• Crear documentos de Word\n• Crear presentaciones de PowerPoint\n• Investigar temas en internet\n• Consejos de Office\n\n¿Qué te gustaría hacer?'

    def _generate_response(self, rule: KnowledgeRule, topic: str) -> str:
        responses = {
            "crear_documento_pentesting": f"Perfecto, para crear un documento sobre pentesting:\n1. Investigaré el tema para obtener información actualizada\n2. Organizaré el contenido con secciones claras\n3. Aplicaré estilos profesionales\n4. Incluiré tablas y referencias\n\n¿Listo para generar el documento?",
            "crear_presentacion_pentesting": f"¡Genial! Para la presentación sobre pentesting:\n1. Usaré un tema atractivo (modern/professional/vibrant\n2. Incluiré slides de introducción, contenido y conclusiones\n3. Aplicaré colores coordinados\n4. Formatearé títulos y texto automáticamente\n\n¿Qué tema prefieres?",
            "investigar_tema": f"Excelente, investigaré sobre '{topic}'\n1. Buscaré fuentes confiables\n2. Extraeré información relevante\n3. Generaré un resumen estructurado\n4. Citaré las fuentes\n\nUn momento mientras busco información...",
            "dar_consejos_office": f"Claro, te daré consejos útiles:\n1. Revisaré mi base de conocimientos\n2. Filtraré tips relevantes\n3. Te los presentaré de forma organizada\n\n¿De qué herramienta necesitas consejos?",
            "explicar_pentesting": f"¡Perfecto! Aquí tienes una explicación estructurada sobre pentesting:\n1. Definición clara\n2. Metodologías principales\n3. Fases del proceso\n4. Herramientas esenciales\n5. Consideraciones éticas\n\nEmpecemos..."
        }
        return responses.get(rule.conclusion, "Entendido, ¿en qué puedo ayudarte?")

    def _general_response(self, user_input: str) -> Tuple[str, List[ReasoningStep]]:
        return ("Puedo ayudarte con:\n• Investigación web\n• Crear documentos de Word\n• Crear presentaciones de PowerPoint\n• Consejos de Office\n• Información sobre pentesting\n\n¿Qué te gustaría hacer?", self.thinking_steps)

    def get_thinking_process(self) -> str:
        return "\n".join([str(step) for step in self.thinking_steps])


class DeductiveReasoner:
    def __init__(self):
        self.premises: List[str] = []

    def add_premise(self, premise: str):
        self.premises.append(premise)

    def deduce(self) -> List[str]:
        conclusions = []
        if all(p in self.premises for p in ["pentesting", "prueba de penetración"]):
            conclusions.append("Se está hablando de seguridad informática")
        if all(p in self.premises for p in ["documento", "word"]):
            conclusions.append("Se necesita un documento de Word")
        if all(p in self.premises for p in ["presentación", "powerpoint"]):
            conclusions.append("Se necesita una presentación de PowerPoint")
        return conclusions


class ChainOfThought:
    def __init__(self):
        self.steps: List[str] = []

    def add_step(self, step: str):
        self.steps.append(step)

    def get_chain(self) -> str:
        return "\n".join([f"{i+1}. {step}" for i, step in enumerate(self.steps)])

    def explain(self) -> str:
        explanation = "Proceso de razonamiento:\n"
        explanation += self.get_chain()
        explanation += "\n\nConclusión final derivada de los pasos anteriores."
        return explanation
