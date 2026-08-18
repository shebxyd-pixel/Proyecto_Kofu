import os
import sys

def get_base_dir():
    if getattr(sys, 'frozen', False):
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        return sys._MEIPASS
    else:
        # Normal execution
        return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))

def get_exec_dir():
    if getattr(sys, 'frozen', False):
        # Directory where the executable is located
        return os.path.dirname(sys.executable)
    else:
        # Normal execution
        return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))

BASE_DIR = get_base_dir()
EXEC_DIR = get_exec_dir()

TEMPLATES_DIR = os.path.join(BASE_DIR, 'templates')
WEB_DIR = os.path.join(BASE_DIR, 'web')
OUTPUT_DIR = os.path.join(EXEC_DIR, 'output')
ARCHIVOS_DIR = os.path.join(EXEC_DIR, 'archivos')

# Ensure user directories exist
os.makedirs(OUTPUT_DIR, exist_ok=True)
os.makedirs(ARCHIVOS_DIR, exist_ok=True)
