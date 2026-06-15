
import os
import torch
from transformers import (
    AutoModelForCausalLM,
    AutoTokenizer,
    BitsAndBytesConfig,
    TrainingArguments,
    Trainer,
    DataCollatorForLanguageModeling
)
from peft import (
    LoraConfig,
    PeftModel,
    get_peft_model,
    prepare_model_for_kbit_training
)
from datasets import load_dataset, Dataset
from typing import Optional, List, Dict, Any
import json


class WebtrestFineTuner:
    def __init__(
        self,
        base_model_name: str = "mistralai/Mistral-7B-Instruct-v0.3",
        output_dir: str = "./models/webtrest-finetuned",
        use_4bit: bool = True
    ):
        self.base_model_name = base_model_name
        self.output_dir = output_dir
        self.use_4bit = use_4bit
        
        os.makedirs(output_dir, exist_ok=True)
        os.makedirs(os.path.join(output_dir, "checkpoints"), exist_ok=True)
        
        self.model = None
        self.tokenizer = None
        self.lora_config = None
        
        print(f"=== Inicializando Fine-Tuner ===")
        print(f"Modelo base: {base_model_name}")
        print(f"Directorio de salida: {output_dir}")
        print(f"Cuantización 4-bit: {use_4bit}")

    def load_base_model(self):
        print("Cargando tokenizer...")
        self.tokenizer = AutoTokenizer.from_pretrained(
            self.base_model_name,
            trust_remote_code=True
        )
        self.tokenizer.pad_token = self.tokenizer.eos_token
        self.tokenizer.padding_side = "right"

        print("Configurando cuantización...")
        if self.use_4bit:
            bnb_config = BitsAndBytesConfig(
                load_in_4bit=True,
                bnb_4bit_use_double_quant=True,
                bnb_4bit_quant_type="nf4",
                bnb_4bit_compute_dtype=torch.bfloat16
            )
        else:
            bnb_config = None

        print("Cargando modelo base...")
        self.model = AutoModelForCausalLM.from_pretrained(
            self.base_model_name,
            quantization_config=bnb_config,
            device_map="auto",
            trust_remote_code=True,
            use_auth_token=True
        )

        if self.use_4bit:
            self.model = prepare_model_for_kbit_training(self.model)

        print("✅ Modelo base cargado!")

    def setup_lora(self, r: int = 8, lora_alpha: int = 32, lora_dropout: float = 0.05):
        print("Configurando LoRA...")
        
        target_modules = [
            "q_proj", "k_proj", "v_proj", "o_proj",
            "gate_proj", "up_proj", "down_proj"
        ]
        
        self.lora_config = LoraConfig(
            r=r,
            lora_alpha=lora_alpha,
            target_modules=target_modules,
            lora_dropout=lora_dropout,
            bias="none",
            task_type="CAUSAL_LM"
        )
        
        self.model = get_peft_model(self.model, self.lora_config)
        
        trainable_params = sum(p.numel() for p in self.model.parameters() if p.requires_grad)
        total_params = sum(p.numel() for p in self.model.parameters())
        print(f"✅ LoRA configurado!")
        print(f"Parámetros entrenables: {trainable_params:,} ({100 * trainable_params / total_params:.2f}%)")

    def prepare_dataset(self, data_file: str, format: str = "json"):
        print(f"Preparando dataset desde: {data_file}")
        
        if format == "json":
            with open(data_file, 'r', encoding='utf-8') as f:
                data = json.load(f)
        elif format == "jsonl":
            data = []
            with open(data_file, 'r', encoding='utf-8') as f:
                for line in f:
                    data.append(json.loads(line.strip()))
        else:
            raise ValueError(f"Formato no soportado: {format}")
        
        def format_example(example):
            prompt = f"[INST] {example['instruction']} [/INST] {example['output']}"
            return {"text": prompt}
        
        dataset = Dataset.from_list(data)
        dataset = dataset.map(format_example, remove_columns=dataset.column_names)
        
        tokenized_dataset = dataset.map(
            lambda examples: self.tokenizer(
                examples["text"],
                truncation=True,
                max_length=512,
                padding="max_length"
            ),
            batched=True,
            remove_columns=["text"]
        )
        
        print(f"✅ Dataset preparado: {len(tokenized_dataset)} ejemplos")
        return tokenized_dataset

    def train(
        self,
        dataset,
        num_train_epochs: int = 3,
        per_device_train_batch_size: int = 4,
        gradient_accumulation_steps: int = 4,
        learning_rate: float = 2e-5,
        logging_steps: int = 10
    ):
        print("=== Iniciando entrenamiento ===")
        
        training_args = TrainingArguments(
            output_dir=os.path.join(self.output_dir, "checkpoints"),
            num_train_epochs=num_train_epochs,
            per_device_train_batch_size=per_device_train_batch_size,
            gradient_accumulation_steps=gradient_accumulation_steps,
            learning_rate=learning_rate,
            logging_steps=logging_steps,
            save_strategy="epoch",
            fp16=True,
            optim="paged_adamw_32bit",
            report_to="none"
        )
        
        data_collator = DataCollatorForLanguageModeling(
            tokenizer=self.tokenizer,
            mlm=False
        )
        
        trainer = Trainer(
            model=self.model,
            args=training_args,
            train_dataset=dataset,
            data_collator=data_collator
        )
        
        trainer.train()
        
        print("✅ Entrenamiento completado!")
        
        final_model_path = os.path.join(self.output_dir, "final")
        self.model.save_pretrained(final_model_path)
        self.tokenizer.save_pretrained(final_model_path)
        
        print(f"✅ Modelo guardado en: {final_model_path}")
        return final_model_path

    def load_finetuned_model(self, finetuned_path: str):
        print(f"Cargando modelo fine-tuneado desde: {finetuned_path}")
        
        self.tokenizer = AutoTokenizer.from_pretrained(finetuned_path)
        
        bnb_config = BitsAndBytesConfig(
            load_in_4bit=True,
            bnb_4bit_use_double_quant=True,
            bnb_4bit_quant_type="nf4",
            bnb_4bit_compute_dtype=torch.bfloat16
        )
        
        base_model = AutoModelForCausalLM.from_pretrained(
            self.base_model_name,
            quantization_config=bnb_config,
            device_map="auto",
            trust_remote_code=True
        )
        
        self.model = PeftModel.from_pretrained(base_model, finetuned_path)
        self.model = self.model.merge_and_unload()
        
        print("✅ Modelo fine-tuneado cargado!")
        return self.model, self.tokenizer


if __name__ == "__main__":
    print("Este módulo se usa para fine-tuning. Ejecuta train_model.py para iniciar.")
