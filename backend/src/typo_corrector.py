import re


class TypoCorrector:
    def __init__(self):
        self.typo_dictionary = {
            "pentesting": ["pentestig", "pentestin", "pentesing", "pentestng", "pentest"],
            "powerpoint": ["powerpoin", "powerpint", "powepoint", "powerpiont", "ppt"],
            "word": ["wrod", "wor", "wrd", "wordd"],
            "documento": ["documeto", "document", "documnto", "doc"],
            "presentacion": ["presentacio", "presentacionn", "presntacion", "ppt"],
            "plantilla": ["plantila", "plantllla", "plantil", "template"],
            "crear": ["crear", "crea", "crar", "hacer"],
            "investigar": ["investigar", "investiga", "investgar", "buscar"],
            "consejos": ["consejos", "consejo", "consejos", "tips"],
            "ayuda": ["ayuda", "aydua", "help"],
            "hola": ["hola", "hol", "ola", "hi"],
            "que": ["que", "q", "ke", "qué"],
            "es": ["es", "e", "ess"],
            "el": ["el", "ell", "le"],
            "la": ["la", "l", "al"],
            "de": ["de", "d", "del"],
            "en": ["en", "n", "em"],
            "y": ["y", "i", "e"],
            "para": ["para", "pra", "par", "pa"],
            "con": ["con", "c", "co"],
            "por": ["por", "p", "po"],
            "los": ["los", "ls", "lo"],
            "las": ["las", "l", "la"],
            "un": ["un", "u", "una"],
            "una": ["una", "u", "un"],
            "se": ["se", "s", "es"],
            "no": ["no", "n", "non"],
            "si": ["si", "s", "sí"],
            "mas": ["mas", "más", "m"],
            "menos": ["menos", "mns", "men"],
            "tambien": ["tambien", "también", "tmb"],
            "muy": ["muy", "mu", "m"],
            "bien": ["bien", "bn", "b"],
            "mal": ["mal", "ml", "m"],
            "todo": ["todo", "td", "t"],
            "nada": ["nada", "nd", "n"]
        }
        
        self.common_phrases = {
            "que es el pentesting": ["q es el pentesting", "ke es el pentesting", "que es pentesting", "que es el pentest"],
            "crear documento de word": ["crear doc de word", "hacer documento de word", "crear word", "crear documento word"],
            "crear presentacion de powerpoint": ["crear ppt", "hacer presentacion powerpoint", "crear presentacion ppt", "crear powerpoint"],
            "consejos de powerpoint": ["tips de powerpoint", "consejos ppt", "ayuda powerpoint"],
            "consejos de word": ["tips de word", "consejos doc", "ayuda word"],
            "investigar sobre": ["buscar sobre", "investiga sobre", "busca sobre"],
            "ayuda": ["ayuda por favor", "necesito ayuda", "help"]
        }
    
    def correct_word(self, word):
        word_lower = word.lower()
        
        for correct_word, typos in self.typo_dictionary.items():
            if word_lower in typos or word_lower == correct_word:
                return correct_word
        
        return word
    
    def correct_text(self, text):
        text_lower = text.lower()
        
        for correct_phrase, phrases in self.common_phrases.items():
            for phrase in phrases:
                if phrase in text_lower:
                    text_lower = text_lower.replace(phrase, correct_phrase)
        
        words = re.findall(r'\w+', text_lower)
        corrected_words = [self.correct_word(word) for word in words]
        
        return ' '.join(corrected_words)
    
    def is_similar(self, word1, word2, threshold=0.8):
        len1, len2 = len(word1), len(word2)
        if abs(len1 - len2) > 2:
            return False
            
        distance = self.levenshtein_distance(word1.lower(), word2.lower())
        max_len = max(len1, len2)
        similarity = 1 - (distance / max_len)
        
        return similarity >= threshold
    
    def levenshtein_distance(self, s1, s2):
        if len(s1) < len(s2):
            return self.levenshtein_distance(s2, s1)
        
        if len(s2) == 0:
            return len(s1)
        
        previous_row = range(len(s2) + 1)
        for i, c1 in enumerate(s1):
            current_row = [i + 1]
            for j, c2 in enumerate(s2):
                insertions = previous_row[j + 1] + 1
                deletions = current_row[j] + 1
                substitutions = previous_row[j] + (c1 != c2)
                current_row.append(min(insertions, deletions, substitutions))
            previous_row = current_row
        
        return previous_row[-1]


if __name__ == "__main__":
    corrector = TypoCorrector()
    
    test_texts = [
        "hola, q es el pentestig?",
        "crea un doc de word por favor",
        "necesito una presentacion de powerpint",
        "dame consejos de ppt",
        "investiga sobre seguridad informatica"
    ]
    
    for text in test_texts:
        corrected = corrector.correct_text(text)
        print(f"Original: {text}")
        print(f"Corregido: {corrected}")
        print("-" * 50)
