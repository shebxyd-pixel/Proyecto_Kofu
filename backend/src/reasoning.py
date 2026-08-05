"""
Motor de razonamiento de Kofu.

- ReasoningEngine: reglas locales, sin dependencias externas, siempre disponible.
- HybridReasoningEngine: intenta reglas + Ollama (modelo dinámico, resuelto por
  request) y, solo en modo 'online', cae a un LLM en la nube si Ollama falla.

Antes (api.py): HybridReasoningEngine se construía con un `model_name` fijo
que se pasaba una sola vez a OllamaEngine al arrancar. Ahora el modelo viaja
como parámetro en cada llamada a reason()/direct_reason(), así que un mismo
proceso puede atender una request con "llama3:latest" y la siguiente con
"gemma4:latest" sin reiniciar nada.
"""
import os
import re
from typing import List, Optional, Tuple

import requests

from ollama_client import OllamaEngine, NoModelAvailableError


class ReasoningStep:
    def __init__(self, step_num: int, thought: str, evidence: str = ""):
        self.step_num, self.thought, self.evidence = step_num, thought, evidence

    def __str__(self):
        return f"Paso {self.step_num}: {self.thought}" + (f" ({self.evidence})" if self.evidence else "")


class ChainOfThought:
    def __init__(self):
        self.steps: List[str] = []

    def add_step(self, step: str):
        self.steps.append(step)

    def get_chain(self) -> str:
        return "\n".join(f"{i + 1}. {s}" for i, s in enumerate(self.steps))

    def explain(self) -> str:
        return f"Proceso de razonamiento:\n{self.get_chain()}\n\nConclusión final derivada de los pasos anteriores."


class KnowledgeRule:
    def __init__(self, name: str, conditions: List[str], conclusion: str):
        self.name, self.conditions, self.conclusion = name, conditions, conclusion


class ReasoningEngine:
    """Motor basado en reglas: rápido, sin dependencias externas, siempre disponible."""

    def __init__(self):
        self.rules = [
            KnowledgeRule("crear_documento_pentesting",
                          ["necesita informacion sobre pentesting", "necesita un documento de seguridad"],
                          "crear_documento_pentesting"),
            KnowledgeRule("crear_presentacion_pentesting",
                          ["necesita una presentacion de pentesting", "presentacion de seguridad"],
                          "crear_presentacion_pentesting"),
            KnowledgeRule("investigar_tema",
                          ["necesita informacion sobre", "quiere saber sobre", "investiga", "busca"],
                          "investigar_tema"),
            KnowledgeRule("dar_consejos_office",
                          ["consejos de", "tips de", "trucos de", "como usar"],
                          "dar_consejos_office"),
            KnowledgeRule("explicar_pentesting", ["que es", "explica", "que son"], "explicar_pentesting"),
        ]
        self.facts: List[str] = []
        self.thinking_steps: List[ReasoningStep] = []

    def add_fact(self, fact: str):
        self.facts.append(fact.lower())

    def clear_facts(self):
        self.facts, self.thinking_steps = [], []

    def _extract_topic(self, text: str) -> str:
        for pattern in (r"sobre\s+(.+?)(?:\?|$|,|\.)", r"de\s+(.+?)(?:\?|$|,|\.)", r"acerca\s+de\s+(.+?)(?:\?|$|,|\.)"):
            match = re.search(pattern, text.lower())
            if match:
                return match.group(1).strip()
        return ""

    def _fallback_response(self, user_input: str) -> str:
        low = user_input.lower()
        if any(w in low for w in ("pentesting", "seguridad", "hacker")):
            return ("Sobre pentesting:\n• Metodologías: OWASP, PTES, OSSTMM, NIST\n"
                    "• Fases: Reconocimiento → Escaneo → Enumeración → Explotación → Post-explotación → Informes\n"
                    "• Herramientas: Nmap, Metasploit, Wireshark, Burp Suite\n\n¿Creo un documento o presentación?")
        if any(w in low for w in ("powerpoint", "presentación", "pptx", "ppt")):
            return "Puedo crear una presentación con temas professional, modern o vibrant. ¿Sobre qué tema?"
        if any(w in low for w in ("word", "documento", "docx")):
            return "Puedo crear un documento Word con estilo professional o modern. ¿Sobre qué tema?"
        return ("Puedo ayudarte con:\n• Pentesting\n• Documentos Word\n• Presentaciones PowerPoint\n"
                "• Investigación web\n• Consejos de Office\n\n¿Qué te gustaría hacer?")

    def reason(self, user_input: str) -> Tuple[str, List[ReasoningStep]]:
        self.clear_facts()
        self.add_fact(user_input)
        self.thinking_steps.append(ReasoningStep(1, "Analizando entrada del usuario", user_input))

        topic = self._extract_topic(user_input)
        if topic:
            self.thinking_steps.append(ReasoningStep(2, f"Tema identificado: {topic}"))

        for rule in self.rules:
            if any(cond in user_input.lower() for cond in rule.conditions):
                self.thinking_steps.append(ReasoningStep(3, f"Regla aplicada: {rule.name}"))
                return self._fallback_response(user_input), self.thinking_steps

        self.thinking_steps.append(ReasoningStep(3, "Ninguna regla específica aplicada"))
        return self._fallback_response(user_input), self.thinking_steps

    def get_thinking_process(self) -> str:
        return "\n".join(str(s) for s in self.thinking_steps)


def query_external_llm_fallback(prompt: str) -> Optional[str]:
    """Fallback opcional a un LLM en la nube (solo si OPENAI_API_KEY está configurada).
    Solo se invoca en modo 'online', y solo si Ollama falló por una razón
    transitoria (no cuando falta el modelo, ver HybridReasoningEngine)."""
    api_key = os.getenv("OPENAI_API_KEY")
    if not api_key:
        return None
    try:
        resp = requests.post(
            "https://api.openai.com/v1/chat/completions",
            headers={"Authorization": f"Bearer {api_key}", "Content-Type": "application/json"},
            json={"model": "gpt-3.5-turbo", "messages": [{"role": "user", "content": prompt}],
                  "max_tokens": 500, "temperature": 0.7},
            timeout=30,
        )
        resp.raise_for_status()
        return resp.json()["choices"][0]["message"]["content"].strip()
    except Exception:
        return None


class HybridReasoningEngine(ReasoningEngine):
    """Reglas locales primero; si Ollama está disponible, lo usa para generar
    respuestas más ricas con el modelo pedido por el frontend en esa request.
    Si Ollama falla por un error transitorio, cae al fallback basado en reglas
    y, en último caso (modo online), a un LLM externo opcional.

    Si Ollama responde pero NO tiene instalado ni el modelo pedido ni ninguno
    de los modelos de fallback (config.DEFAULT_OLLAMA_MODELS), NO se enmascara
    el error: se deja propagar NoModelAvailableError para que la ruta HTTP
    conteste 418 "Error 418; No hay modelo disponible", tal como pide el
    frontend en vez de devolver una respuesta silenciosa basada solo en reglas.
    """

    def __init__(self, use_ollama: bool = True):
        super().__init__()
        self.ollama_engine = OllamaEngine() if use_ollama else None
        self.chain_of_thought = ChainOfThought()
        self.last_model_used: Optional[str] = None

    def _respuesta_con_ollama(self, prompt: str, system_prompt: Optional[str] = None,
                               allow_external: bool = False, model: Optional[str] = None) -> Optional[str]:
        if self.ollama_engine:
            try:
                respuesta, modelo_usado = self.ollama_engine.razonar(prompt, model=model, system_prompt=system_prompt)
                self.last_model_used = modelo_usado
                return respuesta
            except NoModelAvailableError:
                raise  # caso explícito: no se enmascara, sube hasta la ruta HTTP como 418
            except RuntimeError:
                pass  # error transitorio (timeout, conexión): se intenta el fallback normal
        return query_external_llm_fallback(prompt) if allow_external else None

    def direct_reason(self, user_input: str, allow_external: bool = False,
                       model: Optional[str] = None) -> Tuple[str, List[ReasoningStep]]:
        self.clear_facts()
        self.add_fact(user_input)
        self.thinking_steps.append(ReasoningStep(1, "Acceso directo sin sanitización", user_input))

        respuesta = self._respuesta_con_ollama(
            user_input,
            system_prompt="Eres un asistente de razonamiento directo. Responde de forma técnica y concisa.",
            allow_external=allow_external,
            model=model,
        )
        if respuesta:
            self.thinking_steps.append(ReasoningStep(2, "Respuesta generada con IA"))
            return respuesta, self.thinking_steps

        self.thinking_steps.append(ReasoningStep(2, "Respuesta directa sin motor generativo"))
        return user_input, self.thinking_steps

    def reason(self, user_input: str, allow_external: bool = False,
               model: Optional[str] = None) -> Tuple[str, List[ReasoningStep]]:
        respuesta_reglas, pasos = super().reason(user_input)

        respuesta_ia = self._respuesta_con_ollama(user_input, allow_external=allow_external, model=model)
        if respuesta_ia:
            pasos.append(ReasoningStep(len(pasos) + 1, "Respuesta enriquecida con IA generativa"))
            return respuesta_ia, pasos

        return respuesta_reglas, pasos
