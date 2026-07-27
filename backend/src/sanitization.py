
import os
import re


class SanitizadorEntrada:

    PATRONES_PELIGROSOS = [
        r"<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>",
        r"javascript:", r"vbscript:", r"onload=", r"onclick=", r"onerror=",
        r"eval\s*\(", r"exec\s*\(", r"system\s*\(", r"shell\s*\(",
        r"drop\s+table", r"select\s+.*\s+from", r"insert\s+into", r"delete\s+from",
        r"rm\s+-rf", r"mkdir\s+", r"rmdir\s+", r"chmod\s+", r"sudo\s+",
        r"<iframe", r"<object", r"<embed", r"<svg", r"<link", r"<meta",
    ]
    ETIQUETAS_PERMITIDAS = ["b", "i", "u", "strong", "em", "p", "ul", "ol", "li", "h1", "h2", "h3"]

    def __init__(self):
        try:
            import bleach
            self._bleach = bleach
        except ImportError:
            self._bleach = None

    def limpiar_texto(self, texto: str) -> str:
        texto = str(texto)
        for patron in self.PATRONES_PELIGROSOS:
            texto = re.sub(patron, "", texto, flags=re.IGNORECASE)

        if self._bleach:
            texto = self._bleach.clean(
                texto, tags=self.ETIQUETAS_PERMITIDAS, attributes={}, strip=True, strip_comments=True
            )
        else:
            texto = re.sub(r"<[^>]+>", "", texto)

        texto = re.sub(r"[\x00-\x1F\x7F-\x9F]", "", texto)
        return re.sub(r"\s+", " ", texto).strip()

    def es_entrada_segura(self, texto: str) -> bool:
        limpio = self.limpiar_texto(texto)
        return 0 < len(limpio) <= 10000

    def sanitizar_nombre_archivo(self, nombre: str) -> str:
        limpio = re.sub(r'[<>:"/\\|?*]', "", nombre)
        limpio = os.path.basename(limpio)
        return limpio[:255]


class TypoCorrector:
    DICCIONARIO = {
        "powerpoint": ["powerpoin", "powerpint", "powepoint", "powerpiont", "ppt"],
        "word": ["wrod", "wor", "wrd", "wordd"],
        "documento": ["documeto", "document", "documnto", "doc"],
        "presentacion": ["presentacio", "presentacionn", "presntacion"],
        "plantilla": ["plantila", "plantllla", "plantil", "template"],
        "crear": ["crea", "crar", "hacer"],
        "investigar": ["investiga", "investgar", "buscar"],
        "consejos": ["consejo", "tips"],
        "ayuda": ["aydua", "help"],
    }
    FRASES_COMUNES = {
        "crear documento de word": ["crear doc de word", "hacer documento de word", "crear word"],
        "crear presentacion de powerpoint": ["crear ppt", "hacer presentacion powerpoint", "crear powerpoint"],
        "consejos de powerpoint": ["tips de powerpoint", "consejos ppt", "ayuda powerpoint"],
        "consejos de word": ["tips de word", "consejos doc", "ayuda word"],
        "investigar sobre": ["buscar sobre", "investiga sobre", "busca sobre"],
    }

    def correct_word(self, word: str) -> str:
        word_lower = word.lower()
        for correcta, typos in self.DICCIONARIO.items():
            if word_lower in typos or word_lower == correcta:
                return correcta
        return word

    def correct_text(self, text: str) -> str:
        text_lower = text.lower()
        for frase_correcta, variantes in self.FRASES_COMUNES.items():
            for variante in variantes:
                if variante in text_lower:
                    text_lower = text_lower.replace(variante, frase_correcta)
        palabras = re.findall(r"\w+", text_lower)
        return " ".join(self.correct_word(w) for w in palabras)
