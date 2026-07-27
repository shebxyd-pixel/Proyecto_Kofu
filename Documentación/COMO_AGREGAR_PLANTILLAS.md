# Cómo agregar plantillas a Kofu

## 1. Tipos de archivos soportados

- **PowerPoint**: .pptx, .potx, .pptm
- **Word**: .docx, .dotx, .docm

## 2. Ubicación de las plantillas

Coloca tus plantillas en las carpetas correspondientes:
- Plantillas PowerPoint: `templates/powerpoint/`
- Plantillas Word: `templates/word/`

## 3. Pasos para agregar una plantilla

### Paso 1: Crear o editar tu plantilla
1. Abre tu programa de Office (PowerPoint o Word)
2. Crea o abre el documento que quieres usar como plantilla
3. Personaliza el diseño, colores, fuentes, etc.

### Paso 2: Guardar como plantilla
1. Ve a **Archivo → Guardar como**
2. Selecciona la ubicación temporal
3. Elige el formato de plantilla:
   - **PowerPoint**: `Plantilla PowerPoint (*.potx)`
   - **Word**: `Plantilla Word (*.dotx)`
4. Dale un nombre descriptivo
5. Guarda el archivo

### Paso 3: Agregar la plantilla a Kofu
1. Copia el archivo de plantilla guardado
2. Pégalo en la carpeta correspondiente:
   - `templates/powerpoint/` para plantillas de PowerPoint
   - `templates/word/` para plantillas de Word

### Paso 4: Reiniciar el servidor
Si el servidor de Kofu estaba corriendo, reinícialo para que detecte la nueva plantilla.

## 4. Nombres recomendados

Usa nombres descriptivos para tus plantillas, por ejemplo:
- `presentacion_profesional.potx`
- `informe_tecnico.dotx`
- `plantilla_reunion.pptx`
- `plantilla_cv.docx`

## 5. Solución de problemas

### La plantilla no aparece en la lista
- Asegúrate de que está en la carpeta correcta
- Verifica que el formato es válido (.potx/.pptx/.pptm para PowerPoint, .dotx/.docx/.docm para Word)
- Reinicia el servidor

### Error al usar la plantilla
- Asegúrate de que la plantilla no está dañada
- Verifica que Microsoft Office está correctamente instalado en tu sistema
- Prueba a abrir la plantilla directamente en Office para ver si funciona

### Plantilla muy grande o lenta
- Reduce el tamaño de las imágenes
- Elimina elementos innecesarios
- Usa formatos comprimidos
