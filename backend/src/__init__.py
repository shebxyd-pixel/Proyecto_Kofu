"""
Paquete backend de Kofu.
Toda la lógica vive ahora en api.py (sanitización, corrección, razonamiento
híbrido con Ollama, investigación web, generación de Office y digestión de
archivos). Este __init__.py solo re-exporta lo necesario para quien quiera
importar el asistente como librería en vez de usar la API HTTP.
"""

from .api import (
    app,
    AIAssistant,
    HybridReasoningEngine,
    ReasoningEngine,
    ReasoningStep,
    ChainOfThought,
    OllamaEngine,
    OfficeAgent,
    WebResearcher,
    FileProcessor,
    SanitizadorEntrada,
    TypoCorrector,
    KnowledgeBase,
)

__all__ = [
    "app",
    "AIAssistant",
    "HybridReasoningEngine",
    "ReasoningEngine",
    "ReasoningStep",
    "ChainOfThought",
    "OllamaEngine",
    "OfficeAgent",
    "WebResearcher",
    "FileProcessor",
    "SanitizadorEntrada",
    "TypoCorrector",
    "KnowledgeBase",
]
