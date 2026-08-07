from typing import Optional
from pydantic import BaseModel


class ChatRequest(BaseModel):
    message: str
    direct: bool = False
    modo: str = "local"
    model: Optional[str] = None


class TopicRequest(BaseModel):
    topic: str
    modo: str = "online"
    model: Optional[str] = None


class PresentationRequest(BaseModel):
    topic: str
    output_path: Optional[str] = None
    theme: str = "professional"
    modo: str = "online"
    filename: Optional[str] = None
    template: Optional[str] = None
    model: Optional[str] = None


class DocumentRequest(BaseModel):
    topic: str
    output_path: Optional[str] = None
    style: str = "professional"
    modo: str = "online"
    filename: Optional[str] = None
    template: Optional[str] = None
    model: Optional[str] = None


class TipsRequest(BaseModel):
    software: str


class DigestRequest(BaseModel):
    file_path: str
    output_path: Optional[str] = None
    instrucciones: Optional[str] = None
    model: Optional[str] = None


class CorrectRequest(BaseModel):
    text: str
    