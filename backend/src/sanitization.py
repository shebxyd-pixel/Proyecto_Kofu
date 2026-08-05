import re

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
