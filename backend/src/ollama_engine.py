import os
import json
import requests
from typing import Tuple, List, Dict, Any
from .reasoning_engine import ReasoningEngine, ReasoningStep, ChainOfThought


class OllamaEngine:
    def __init__(self, model_name: str = None):
        self.base_url = os.getenv("OLLAMA_BASE_URL", "http://localhost:11434")
        self.model_name = (model_name or os.getenv("OLLAMA_MODEL") or os.getenv("LLM_MODEL_NAME") or "gemma4")
        self.available = self._check_availability()

    def _check_availability(self) -> bool:
        try:
            response = requests.get(f"{self.base_url}/api/tags", timeout=2)
            if response.status_code == 200:
                print(f"✅ Conexión a Ollama exitosa! Modelos disponibles: {[m['name'] for m in response.json()['models']]}")
                return True
            else:
                print(f"⚠️ Ollama respondió pero hubo un problema")
                return False
        except Exception as e:
            print(f"⚠️ No se pudo conectar a Ollama: {e}")
            return False

    def sanitizar_entrada(self, texto_sucio: str) -> Tuple[Dict[str, Any], bool]:
        """
        Sanitiza la entrada usando Ollama con Gemma/el modelo configurado
        """
        if not self.available:
            return {"texto_limpio": texto_sucio, "entidades_detectadas": [], "proceso_exitoso": False}, False

        system_prompt = """
        Eres un motor de sanitización avanzada para datos estructurados.
        Tu única función es procesar la entrada proporcionada, limpiar cualquier ruido,
        y devolver ÚNICAMENTE un JSON válido que contenga los siguientes campos:
        'texto_limpio', 'entidades_detectadas' y 'proceso_exitoso'.
        El campo 'texto_limpio' debe ser el texto procesado sin saltos de línea extra o caracteres irrelevantes.
        """

        data = {
            "model": self.model_name,
            "prompt": f"{system_prompt}\n\nDATOS A PROCESAR: '{texto_sucio}'",
            "stream": False,
            "options": {
                "temperature": 0.1,
                "num_predict": 512
            }
        }

        try:
            response = requests.post(
                f"{self.base_url}/api/generate",
                headers={"Content-Type": "application/json"},
                data=json.dumps(data),
                timeout=60
            )
            response.raise_for_status()
            response_json = response.json()
            resultado_raw = response_json["response"].strip()

            try:
                return json.loads(resultado_raw), True
            except json.JSONDecodeError:
                print("Advertencia: El modelo no devolvió JSON perfectamente. Devolviendo texto raw.")
                return {"texto_limpio": resultado_raw, "entidades_detectadas": [], "proceso_exitoso": False}, False

        except requests.exceptions.ConnectionError:
            print("ERROR DE CONEXIÓN: Asegúrate de que Ollama esté corriendo en segundo plano y el servicio API esté activo.")
            return {"texto_limpio": texto_sucio, "entidades_detectadas": [], "proceso_exitoso": False}, False
        except Exception as e:
            print(f"Error al sanitizar: {e}")
            return {"texto_limpio": texto_sucio, "entidades_detectadas": [], "proceso_exitoso": False}, False

    def razonar(self, prompt: str, system_prompt: str = None) -> str:
        """
        Realiza razonamiento offline usando Ollama
        """
        if not self.available:
            raise Exception("Ollama no está disponible")

        system_msg = system_prompt or "Eres Kofu, un asistente de IA para crear documentos y presentaciones. Responde de forma clara y útil."

        data = {
            "model": self.model_name,
            "prompt": f"{system_msg}\n\nUSUARIO: {prompt}",
            "stream": False,
            "options": {
                "temperature": 0.7,
                "num_predict": 1024
            }
        }

        try:
            response = requests.post(
                f"{self.base_url}/api/generate",
                headers={"Content-Type": "application/json"},
                data=json.dumps(data),
                timeout=120
            )
            response.raise_for_status()
            response_json = response.json()
            return response_json["response"].strip()
        except Exception as e:
            raise Exception(f"Error al comunicarse con Ollama: {e}")


class HybridReasoningEngine(ReasoningEngine):
    def __init__(self, use_ollama: bool = True, model_name: str = None):
        super().__init__()
        self.ollama_engine = OllamaEngine(model_name=model_name) if use_ollama else None
        self.chain_of_thought = ChainOfThought()

    def direct_reason(self, user_input: str) -> Tuple[str, List[ReasoningStep]]:
        self.thinking_steps = []
        self.chain_of_thought.steps = []
        self.clear_facts()
        self.add_fact(user_input)

        self.thinking_steps.append(ReasoningStep(1, "Acceso directo sin sanitización", user_input))
        self.chain_of_thought.add_step("Acceso directo sin sanitización")

        # if self.ollama_engine and self.ollama_engine.available:
        #     self.thinking_steps.append(ReasoningStep(2, "Generando respuesta directa con Ollama"))
        #     self.chain_of_thought.add_step("Generando respuesta directa con Ollama")

        #     response = self.ollama_engine.razonar(
        #         user_input,
        #         system_prompt="Eres un asistente de razonamiento directo. Responde de forma técnica, sin usar sanitización ni diccionarios externos."
        #     )
        #     self.thinking_steps.append(ReasoningStep(3, "Respuesta directa generada"))
        #     self.chain_of_thought.add_step("Respuesta directa generada")
        #     return response, self.thinking_steps

        response = user_input
        self.thinking_steps.append(ReasoningStep(2, "Respuesta directa sin motor de razonamiento", user_input))
        self.chain_of_thought.add_step("Respuesta directa sin motor de razonamiento")
        return response, self.thinking_steps

    def reason(self, user_input: str) -> Tuple[str, List[ReasoningStep]]:
        self.thinking_steps = []
        self.chain_of_thought.steps = []
        self.clear_facts()
        self.add_fact(user_input)

        self.thinking_steps.append(ReasoningStep(1, "Analizando entrada del usuario", user_input))
        self.chain_of_thought.add_step("Analizando entrada del usuario")

        topic = self._extract_topic(user_input)
        if topic:
            self.thinking_steps.append(ReasoningStep(2, f"Tema identificado: {topic}"))
            self.chain_of_thought.add_step(f"Tema identificado: {topic}")

        try:
            # if self.ollama_engine and self.ollama_engine.available:
            #     self.thinking_steps.append(ReasoningStep(3, "Sanitando entrada con Ollama", f"Modelo: {self.ollama_engine.model_name}"))
            #     self.chain_of_thought.add_step(f"Sanitando entrada con Ollama usando {self.ollama_engine.model_name}")
            #     
            #     sanitizado, exito = self.ollama_engine.sanitizar_entrada(user_input)
            #     texto_limpio = sanitizado.get("texto_limpio", user_input)
            #     
            #     if exito:
            #         self.thinking_steps.append(ReasoningStep(4, "Sanitización completada", "Entrada procesada correctamente"))
            #         self.chain_of_thought.add_step("Sanitización completada")
            #     else:
            #         self.thinking_steps.append(ReasoningStep(4, "Sanitización parcial", "Usando texto limpio sin JSON"))
            #         self.chain_of_thought.add_step("Sanitización parcial: devolviendo texto limpio sin JSON")

            #     self.thinking_steps.append(ReasoningStep(5, "Generando respuesta con IA avanzada (Ollama)"))
            #     self.chain_of_thought.add_step("Generando respuesta con IA avanzada (Ollama)")
            #     response = self.ollama_engine.razonar(texto_limpio)
            #     self.thinking_steps.append(ReasoningStep(6, "Respuesta final generada"))
            #     self.chain_of_thought.add_step("Respuesta final generada")
            #     
            #     return response, self.thinking_steps
            # else:
            self.thinking_steps.append(ReasoningStep(3, "Motor de razonamiento deshabilitado", user_input))
            self.chain_of_thought.add_step("Motor de razonamiento deshabilitado")
            return user_input, self.thinking_steps
        except Exception as e:
            # self.thinking_steps.append(ReasoningStep(3, f"Error con Ollama, usando razonamiento básico", str(e)))
            # self.chain_of_thought.add_step(f"Error con Ollama: {e}. Usando razonamiento básico")
            return user_input, self.thinking_steps

    def get_chain_of_thought(self) -> str:
        return self.chain_of_thought.get_chain()
