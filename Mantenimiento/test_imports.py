import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

try:
    from src import AIAssistant, TypoCorrector, ExternalAIAPI, OfficeAgent
    print("✅ Imports OK: AIAssistant, TypoCorrector, ExternalAIAPI, OfficeAgent")
    
    assistant = AIAssistant()
    print("✅ AIAssistant inicializado correctamente")
    
    corrector = TypoCorrector()
    test = corrector.correct_text("hola, q es el pentestig?")
    print(f"✅ TypoCorrector funciona: {test}")
    
    print("\n✅ Todo funciona correctamente!")
    
except Exception as e:
    print(f"❌ Error: {e}")
    import traceback
    traceback.print_exc()
