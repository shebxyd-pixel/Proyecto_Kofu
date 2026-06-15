
import json
import os
from typing import List, Dict, Any


class TrainingDataPreparer:
    def __init__(self, output_dir: str = "./data/training"):
        self.output_dir = output_dir
        os.makedirs(output_dir, exist_ok=True)
        
        print(f"=== Preparador de Datos de Entrenamiento ===")
        print(f"Directorio de salida: {output_dir}")

    def create_webtrest_dataset(self):
        print("Creando dataset básico de Webtrest...")
        
        dataset = []
        
        # === Datos de pentesting ===
        pentesting_data = [
            {
                "instruction": "¿Qué es el pentesting?",
                "output": "El pentesting (prueba de penetración) es una evaluación de seguridad informática que simula ataques maliciosos para identificar vulnerabilidades en sistemas, redes o aplicaciones web. Su objetivo es encontrar fallos antes que los hackers reales lo hagan."
            },
            {
                "instruction": "Menciona las fases del pentesting",
                "output": "Las fases principales del pentesting son:\n1. Reconocimiento: Recolectar información sobre el objetivo\n2. Escaneo: Identificar puertos, servicios y vulnerabilidades\n3. Enumeración: Obtener detalles más profundos\n4. Explotación: Probar las vulnerabilidades encontradas\n5. Post-explotación: Mantener acceso y analizar impacto\n6. Informes: Documentar hallazgos y recomendaciones"
            },
            {
                "instruction": "¿Qué herramientas se usan en pentesting?",
                "output": "Algunas herramientas populares de pentesting son:\n- Nmap: Escaneo de puertos y redes\n- Metasploit: Explotación de vulnerabilidades\n- Wireshark: Análisis de tráfico de red\n- Burp Suite: Pruebas de aplicaciones web\n- Hydra: Ataques de fuerza bruta\n- John the Ripper: Descifrado de contraseñas"
            }
        ]
        dataset.extend(pentesting_data)
        
        # === Datos de Office ===
        office_data = [
            {
                "instruction": "¿Cómo hacer una buena presentación de PowerPoint?",
                "output": "Para una buena presentación de PowerPoint:\n1. Usa la regla de 6x6: máximo 6 líneas por diapositiva y 6 palabras por línea\n2. Aplica temas consistentes para mantener el diseño profesional\n3. Usa transiciones moderadas para no distraer\n4. Las animaciones deben ser funcionales, no decorativas\n5. Incluye notas del orador para información adicional"
            },
            {
                "instruction": "Consejos para documentos de Word",
                "output": "Consejos para documentos de Word:\n1. Usa estilos predefinidos para títulos y párrafos\n2. Aplica sangría francesa para listas de referencias\n3. Usa la función de navegación para moverte rápidamente\n4. Habilita la revisión de cambios para colaboraciones\n5. Usa plantillas (.dotx) para mantener la coherencia"
            }
        ]
        dataset.extend(office_data)
        
        # === Datos de conversación general ===
        chat_data = [
            {
                "instruction": "Hola, ¿quién eres?",
                "output": "¡Hola! Soy Webtrest, tu asistente de IA especializado en pentesting, generación de documentos de Office (PowerPoint y Word) e investigación web. ¿En qué puedo ayudarte?"
            },
            {
                "instruction": "¿Qué puedes hacer?",
                "output": "Puedo ayudarte con:\n• Información sobre pentesting y seguridad informática\n• Crear presentaciones de PowerPoint\n• Crear documentos de Word\n• Investigar temas en internet\n• Dar consejos sobre herramientas de Office\n\n¿Qué te gustaría hacer hoy?"
            }
        ]
        dataset.extend(chat_data)
        
        output_file = os.path.join(self.output_dir, "webtrest_dataset.json")
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(dataset, f, ensure_ascii=False, indent=2)
        
        print(f"✅ Dataset creado: {len(dataset)} ejemplos")
        print(f"Archivo: {output_file}")
        return output_file

    def add_custom_examples(self, examples: List[Dict[str, str]], existing_file: str = None):
        if existing_file and os.path.exists(existing_file):
            with open(existing_file, 'r', encoding='utf-8') as f:
                dataset = json.load(f)
        else:
            dataset = []
        
        dataset.extend(examples)
        
        output_file = existing_file or os.path.join(self.output_dir, "custom_dataset.json")
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(dataset, f, ensure_ascii=False, indent=2)
        
        print(f"✅ Ejemplos agregados. Total: {len(dataset)}")
        return output_file

    def convert_to_jsonl(self, json_file: str):
        with open(json_file, 'r', encoding='utf-8') as f:
            dataset = json.load(f)
        
        jsonl_file = json_file.replace('.json', '.jsonl')
        with open(jsonl_file, 'w', encoding='utf-8') as f:
            for example in dataset:
                f.write(json.dumps(example, ensure_ascii=False) + '\n')
        
        print(f"✅ Convertido a JSONL: {jsonl_file}")
        return jsonl_file


if __name__ == "__main__":
    preparer = TrainingDataPreparer()
    preparer.create_webtrest_dataset()
