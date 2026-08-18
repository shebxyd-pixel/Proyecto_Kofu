import os
from typing import List, Optional

from fastapi import APIRouter, HTTPException, UploadFile, File
from fastapi.responses import FileResponse

from assistant import AIAssistant
from config import NO_MODEL_ERROR_CODE, NO_MODEL_ERROR_MESSAGE
from ollama_client import NoModelAvailableError
from schemas import (
    ChatRequest,
    CorrectRequest,
    DigestRequest,
    DocumentRequest,
    PresentationRequest,
    TipsRequest,
    TopicRequest,
)

router = APIRouter()

from paths import BASE_DIR, TEMPLATES_DIR, ARCHIVOS_DIR

assistant = AIAssistant()


@router.get("/health")
def health():
    ollama = assistant.reasoning_engine.ollama_engine
    return {
        "status": "ok",
        "ollama_available": ollama.available if ollama else False,
        "ollama_modelos_instalados": ollama.listar_modelos() if ollama else [],
        "ollama_ultimo_modelo_usado": assistant.reasoning_engine.last_model_used,
        "office_templates_available": assistant.office_agent._win32_available,
        "file_processor_available": assistant.file_processor.available,
        "modos_disponibles": ["local", "online"],
    }


@router.get("/ollama/models")
def ollama_models():
    ollama = assistant.reasoning_engine.ollama_engine
    if not ollama:
        return {"models": []}
    return {"models": ollama.listar_modelos()}


@router.get("/templates")
def get_templates():
    ppt_templates: List[str] = []
    word_templates: List[str] = []

    ppt_path = os.path.join(TEMPLATES_DIR, "powerpoint")
    if os.path.exists(ppt_path):
        ppt_templates = [f for f in os.listdir(ppt_path)
                         if f.endswith(('.pptx', '.potx', '.pptm')) and not f.startswith('~$')]

    word_path = os.path.join(TEMPLATES_DIR, "word")
    if os.path.exists(word_path):
        word_templates = [f for f in os.listdir(word_path)
                          if f.endswith(('.docx', '.dotx', '.docm')) and not f.startswith('~$')]

    return {"powerpoint": ppt_templates, "word": word_templates}


@router.post("/chat")
def chat(req: ChatRequest):
    try:
        response, steps = (
            assistant.direct_reason(req.message, modo=req.modo, model=req.model) if req.direct
            else assistant.process_request(req.message, modo=req.modo, model=req.model)
        )
        return {
            "response": response,
            "steps": [str(s) for s in steps],
            "modo": req.modo,
            "model_usado": assistant.reasoning_engine.last_model_used,
        }
    except NoModelAvailableError:
        raise HTTPException(status_code=NO_MODEL_ERROR_CODE, detail=NO_MODEL_ERROR_MESSAGE)
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/research")
def research(req: TopicRequest):
    try:
        return {"summary": assistant.research_topic(req.topic, modo=req.modo, model=req.model)}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/office/powerpoint")
def create_powerpoint(req: PresentationRequest):
    try:
        result = assistant.create_presentation(
            req.topic,
            output_path=req.output_path,
            theme=req.theme,
            modo=req.modo,
            filename=req.filename,
            template=req.template,
            model=req.model,
        )
        backup_name = os.path.basename(result.get("backup_path", "")) if result.get("backup_path") else None
        return {
            "success": True,
            "file_path": result["file_path"],
            "filename": result["filename"],
            "backup_path": result.get("backup_path"),
            "backup_saved": result.get("backup_path") is not None,
            "backup_message": f"✅ Copia guardada en Archivos/{backup_name}" if backup_name else None,
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/office/powerpoint/download")
def download_powerpoint(req: PresentationRequest):
    try:
        result = assistant.create_presentation(
            req.topic,
            output_path=req.output_path,
            theme=req.theme,
            modo=req.modo,
            filename=req.filename,
            template=req.template,
            model=req.model,
        )
        file_path = result["file_path"]
        return FileResponse(
            path=file_path,
            media_type="application/vnd.openxmlformats-officedocument.presentationml.presentation",
            filename=os.path.basename(file_path),
            headers={
                "X-Backup-Saved": "true" if result.get("backup_path") else "false",
                "X-Backup-Path": result.get("backup_path", ""),
            },
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/office/word")
def create_word(req: DocumentRequest):
    try:
        result = assistant.create_document(
            req.topic,
            output_path=req.output_path,
            style=req.style,
            modo=req.modo,
            filename=req.filename,
            template=req.template,
            model=req.model,
        )
        backup_name = os.path.basename(result.get("backup_path", "")) if result.get("backup_path") else None
        return {
            "success": True,
            "file_path": result["file_path"],
            "filename": result["filename"],
            "backup_path": result.get("backup_path"),
            "backup_saved": result.get("backup_path") is not None,
            "backup_message": f"✅ Copia guardada en Archivos/{backup_name}" if backup_name else None,
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/office/word/download")
def download_word(req: DocumentRequest):
    try:
        result = assistant.create_document(
            req.topic,
            output_path=req.output_path,
            style=req.style,
            modo=req.modo,
            filename=req.filename,
            template=req.template,
            model=req.model,
        )
        file_path = result["file_path"]
        return FileResponse(
            path=file_path,
            media_type="application/vnd.openxmlformats-officedocument.wordprocessingml.document",
            filename=os.path.basename(file_path),
            headers={
                "X-Backup-Saved": "true" if result.get("backup_path") else "false",
                "X-Backup-Path": result.get("backup_path", ""),
            },
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/office/tips")
def office_tips(req: TipsRequest):
    return {"tips": assistant.get_office_tips(req.software)}


@router.post("/files/digest")
def digest_file(req: DigestRequest):
    if not assistant.file_processor.available:
        raise HTTPException(status_code=503, detail="MarkItDown no está instalado. Ejecuta: pip install markitdown")
    try:
        resultado = assistant.digest_file(req.file_path, req.output_path, req.instrucciones, model=req.model)
    except NoModelAvailableError:
        raise HTTPException(status_code=NO_MODEL_ERROR_CODE, detail=NO_MODEL_ERROR_MESSAGE)
    if not resultado.get("success"):
        raise HTTPException(status_code=422, detail=resultado.get("error", "No se pudo procesar el archivo."))
    return resultado


@router.post("/text/correct")
def correct_text(req: CorrectRequest):
    sanitizado = (req.text if assistant.sanitizador.es_entrada_segura(req.text)
                  else assistant.sanitizador.limpiar_texto(req.text))
    corregido = assistant.typo_corrector.correct_text(sanitizado)
    return {"original": req.text, "sanitizado": sanitizado, "corregido": corregido}

@router.post("/files/upload")
async def upload_file(file: UploadFile = File(...)):
    # Guardar en Archivos temporalmente
    os.makedirs(ARCHIVOS_DIR, exist_ok=True)
    file_path = os.path.join(ARCHIVOS_DIR, file.filename)
    try:
        with open(file_path, "wb") as buffer:
            buffer.write(await file.read())
        
        # Procesar
        if not assistant.file_processor.available:
            raise HTTPException(status_code=503, detail="MarkItDown no está instalado. Ejecuta: pip install markitdown")
        
        result = assistant.file_processor.process_local_file(file_path)
        if not result.get("success"):
            raise HTTPException(status_code=422, detail=result.get("error", "No se pudo procesar el archivo."))
            
        return result
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

    