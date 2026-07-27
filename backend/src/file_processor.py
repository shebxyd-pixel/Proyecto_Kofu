import os
from typing import Any, Dict


class FileProcessor:
    SUPPORTED_EXTENSIONS = {
        ".pdf", ".docx", ".pptx", ".xlsx", ".xls", ".csv", ".txt", ".md",
        ".html", ".htm", ".xml", ".json", ".yaml", ".yml", ".rtf",
        ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff",
        ".wav", ".mp3", ".m4a", ".ogg", ".flac", ".epub", ".zip", ".msg", ".ipynb",
    }

    def __init__(self):
        try:
            from markitdown import MarkItDown
            self._markitdown = MarkItDown(enable_plugins=False)
        except ImportError:
            self._markitdown = None

    @property
    def available(self) -> bool:
        return self._markitdown is not None

    def process_local_file(self, file_path: str) -> Dict[str, Any]:
        if not self.available:
            return {"success": False, "error": "MarkItDown no está instalado.", "filename": os.path.basename(file_path)}
        try:
            result = self._markitdown.convert_local(file_path)
            return {
                "success": True,
                "text_content": result.text_content,
                "filename": os.path.basename(file_path),
                "file_type": os.path.splitext(file_path)[1].lstrip(".") or "unknown",
            }
        except Exception as e:
            return {"success": False, "error": str(e), "filename": os.path.basename(file_path)}

    @classmethod
    def is_supported_file(cls, filename: str) -> bool:
        return os.path.splitext(filename)[1].lower() in cls.SUPPORTED_EXTENSIONS
