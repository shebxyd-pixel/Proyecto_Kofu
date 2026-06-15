
# Entrenamiento de Webtrest - Guía Completa

Esta guía te enseñará cómo **entrenar/fine-tunear** tu propia versión de Webtrest usando LoRA/QLoRA (eficiente para GPUs locales).

---

## 📋 Requisitos Previos

### Hardware
- **GPU NVIDIA recomendada**: 8 GB VRAM mínimo (16 GB+ para mejor rendimiento)
- **CPU**: 8 núcleos mínimo
- **RAM**: 16 GB mínimo (32 GB recomendado)

### Software
- Python 3.11+
- CUDA 11.8+ (para GPU NVIDIA)
- PyTorch con soporte CUDA

---

## 🚀 Inicio Rápido

### 1. Instalar Dependencias Completas
```bash
pip install -r requirements.txt
```

### 2. Preparar Dataset de Entrenamiento
```bash
python train_model.py --prepare_data
```

Esto creará un dataset básico en `./data/training/webtrest_dataset.json`

### 3. Entrenar el Modelo
```bash
python train_model.py --train
```

---

## 📁 Estructura de Archivos

```
webtrest/
├── src/
│   ├── fine_tuning.py          # Motor de fine-tuning
│   └── data_preparation.py     # Preparación de datos
├── data/
│   └── training/               # Datasets de entrenamiento
│       └── webtrest_dataset.json
├── models/
│   └── webtrest-finetuned/     # Modelos entrenados
│       ├── checkpoints/
│       └── final/
└── train_model.py               # Script principal de entrenamiento
```

---

## 🎯 Uso Detallado

### Preparar Datos
```bash
# Crear dataset básico
python train_model.py --prepare_data

# O usar un dataset personalizado
python train_model.py --data_file ./data/training/mi_dataset.json
```

### Entrenar con Parámetros Personalizados
```bash
python train_model.py \
    --train \
    --base_model mistralai/Mistral-7B-Instruct-v0.3 \
    --output_dir ./models/mi-modelo \
    --epochs 5 \
    --batch_size 8
```

| Parámetro | Descripción | Valor Predeterminado |
|-----------|-------------|-----------------------|
| `--base_model` | Modelo base a usar | `mistralai/Mistral-7B-Instruct-v0.3` |
| `--output_dir` | Directorio para guardar el modelo | `./models/webtrest-finetuned` |
| `--epochs` | Número de épocas | `3` |
| `--batch_size` | Tamaño de lote | `4` |
| `--data_file` | Ruta al dataset | `./data/training/webtrest_dataset.json` |

---

## 📝 Crear tu Propio Dataset

El dataset debe ser un archivo JSON con el siguiente formato:
```json
[
  {
    "instruction": "Pregunta o instrucción del usuario",
    "output": "Respuesta correcta del modelo"
  },
  {
    "instruction": "¿Qué es el pentesting?",
    "output": "El pentesting es una evaluación de seguridad..."
  }
]
```

### Agregar Ejemplos Personalizados
Puedes usar el módulo `TrainingDataPreparer`:
```python
from src.data_preparation import TrainingDataPreparer

preparer = TrainingDataPreparer()

nuevos_ejemplos = [
    {
        "instruction": "Tu pregunta aquí",
        "output": "Tu respuesta aquí"
    }
]

preparer.add_custom_examples(
    nuevos_ejemplos,
    existing_file="./data/training/webtrest_dataset.json"
)
```

---

## 🔧 Tecnologías Usadas

- **LoRA (Low-Rank Adaptation)**: Ahorra memoria entrenando solo un 0.1-5% de los parámetros
- **QLoRA**: LoRA + cuantización 4-bit para GPUs con poca VRAM
- **Hugging Face Transformers**: Biblioteca para modelos
- **PEFT**: Parameter-Efficient Fine-Tuning
- **Datasets**: Manejo de datos de entrenamiento

---

## ⚙️ Optimización para tu GPU

| VRAM Disponible | Configuración Recomendada |
|-----------------|-----------------------------|
| 8-12 GB | QLoRA (4-bit), batch_size=2, r=8 |
| 12-24 GB | QLoRA (4-bit), batch_size=4-8, r=16 |
| 24-48 GB | LoRA (8-bit), batch_size=8-16, r=32 |
| 48 GB+ | Full fine-tuning (si es necesario) |

---

## 📊 Monitoreo del Entrenamiento

Durante el entrenamiento verás:
- Progreso por épocas
- Pérdida (loss)
- Checkpoints guardados por época
- Uso de memoria GPU

---

## 🎉 Usar tu Modelo Entrenado

Después del entrenamiento, tu modelo estará en `./models/webtrest-finetuned/final/`

Para usarlo, carga el modelo con `WebtrestFineTuner`:
```python
from src.fine_tuning import WebtrestFineTuner

tuner = WebtrestFineTuner()
model, tokenizer = tuner.load_finetuned_model("./models/webtrest-finetuned/final")
```

---

## ❓ Preguntas Frecuentes

**¿Puedo entrenar sin GPU?**  
Sí, pero será extremadamente lento. No recomendado.

**¿Qué modelos base puedo usar?**  
- Mistral 7B Instruct
- LLaMA 3.1 8B/70B
- Qwen 2.5 7B
- Zephyr 7B

**¿Cuánto tiempo tarda el entrenamiento?**  
Depende de tu GPU:
- RTX 3090/4090: ~30-60 minutos por época
- RTX 3060/3070: ~2-4 horas por época

---

## 📚 Recursos Adicionales

- [Documentación de PEFT](https://huggingface.co/docs/peft)
- [LoRA Paper](https://arxiv.org/abs/2106.09685)
- [QLoRA Paper](https://arxiv.org/abs/2305.14314)
