import json
from typing import Any, Dict, List, Optional, Tuple

import requests

from config import (
    DEFAULT_OLLAMA_URL,
    DEFAULT_OLLAMA_MODELS,
    OLLAMA_TIMEOUT_CHECK,
    OLLAMA_TIMEOUT_GENERATE,
    NO_MODEL_ERROR_MESSAGE,
)


class NoModelAvailableError(RuntimeError):

    def __init__(self, message: str = NO_MODEL_ERROR_MESSAGE):
        self.message = message
        super().__init__(message)


class OllamaEngine:

    def __init__(self, base_url: Optional[str] = None, timeout: int = OLLAMA_TIMEOUT_CHECK):
        self.base_url = (base_url or DEFAULT_OLLAMA_URL).rstrip("/")
        self.timeout = timeout

    @property
    def available(self) -> bool:
        return bool(self._fetch_available_models())

    def _fetch_available_models(self) -> List[str]:
        try:
            resp = requests.get(f"{self.base_url}/api/tags", timeout=self.timeout)
            resp.raise_for_status()
            return [m.get("name", "") for m in resp.json().get("models", [])]
        except Exception:
            return []

    def listar_modelos(self) -> List[str]:
        return self._fetch_available_models()

    def resolve_model(self, requested_model: Optional[str] = None) -> str:
        available_models = self._fetch_available_models()
        if not available_models:
            raise NoModelAvailableError()

        requested_model = (requested_model or "").strip()
        if requested_model and requested_model in available_models:
            return requested_model

        for candidato in DEFAULT_OLLAMA_MODELS:
            if candidato in available_models:
                return candidato

        raise NoModelAvailableError()

    def _generate(self, prompt: str, model: str, temperature: float = 0.7,
                   num_predict: int = 1024, timeout: int = OLLAMA_TIMEOUT_GENERATE) -> str:
        payload = {
            "model": model, "prompt": prompt, "stream": False,
            "options": {"temperature": temperature, "num_predict": num_predict},
        }
        try:
            resp = requests.post(f"{self.base_url}/api/generate", json=payload, timeout=timeout)
            resp.raise_for_status()
            return resp.json()["response"].strip()
        except requests.exceptions.ConnectionError as e:
            raise RuntimeError("Ollama no responde. Verifica que el servicio esté corriendo.") from e
        except requests.exceptions.Timeout as e:
            raise RuntimeError(f"Tiempo de espera agotado tras {timeout}s.") from e
        except (KeyError, json.JSONDecodeError) as e:
            raise RuntimeError(f"Respuesta inesperada de Ollama: {e}") from e

    def razonar(self, prompt: str, model: Optional[str] = None,
                system_prompt: Optional[str] = None) -> Tuple[str, str]:
        modelo_resuelto = self.resolve_model(model)
        system_msg = system_prompt or (
            "Eres Kofu, un asistente de IA para crear documentos y presentaciones. "
            "Responde de forma clara y útil."
        )
        respuesta = self._generate(f"{system_msg}\n\nUSUARIO: {prompt}", model=modelo_resuelto)
        return respuesta, modelo_resuelto

    def sanitizar_entrada(self, texto_sucio: str, model: Optional[str] = None) -> Tuple[Dict[str, Any], bool]:
        fallback = {"texto_limpio": texto_sucio, "entidades_detectadas": [], "proceso_exitoso": False}
        try:
            modelo_resuelto = self.resolve_model(model)
        except NoModelAvailableError:
            return fallback, False
        prompt = (
            "Devuelve ÚNICAMENTE un JSON con los campos 'texto_limpio', 'entidades_detectadas' "
            f"y 'proceso_exitoso'.\n\nDATOS: '{texto_sucio}'"
        )
        try:
            raw = self._generate(prompt, model=modelo_resuelto, temperature=0.1, num_predict=512, timeout=60)
            return json.loads(raw), True
        except (RuntimeError, json.JSONDecodeError):
            return fallback, False
