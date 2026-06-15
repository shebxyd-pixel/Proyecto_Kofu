import win32com.client as win32
import os


def create_powerpoint_template(template_name, theme_index, output_path):
    try:
        ppt_app = win32.Dispatch("PowerPoint.Application")
        ppt_app.Visible = True

        presentation = ppt_app.Presentations.Add()

        if theme_index is not None:
            presentation.ApplyTheme(theme_index)

        slide_layout = presentation.SlideMaster.CustomLayouts(1)
        slide = presentation.Slides.AddSlide(1, slide_layout)

        output_path = os.path.abspath(output_path)

        presentation.SaveAs(output_path, 27)
        presentation.Close()
        ppt_app.Quit()

        print(f"Plantilla de PowerPoint creada: {output_path}")
        return True
    except Exception as e:
        print(f"Error al crear plantilla {template_name}: {e}")
        return False


def create_custom_templates():
    print("=== Creando plantillas de PowerPoint (.POTX) ===\n")

    templates_folder = os.path.abspath("templates/powerpoint")
    if not os.path.exists(templates_folder):
        os.makedirs(templates_folder)

    templates = [
        {
            "name": "Madison",
            "theme": "Madison",
            "filename": "plantilla_madison.potx"
        },
        {
            "name": "Horizonte Monocromatico",
            "theme": "Monochrome",
            "filename": "plantilla_horizonte_monocromatico.potx"
        },
        {
            "name": "Coleccion Cuadratica",
            "theme": "Quadratic",
            "filename": "plantilla_coleccion_cuadratica.potx"
        }
    ]

    try:
        ppt_app = win32.Dispatch("PowerPoint.Application")
        ppt_app.Visible = True

        for i, template_info in enumerate(templates):
            try:
                print(f"Creando plantilla {i+1}: {template_info['name']}...")

                presentation = ppt_app.Presentations.Add()

                theme_name = template_info['theme']

                master = presentation.SlideMaster

                if theme_name == "Madison":
                    master.Shapes.Title.TextFrame.TextRange.Font.Name = "Segoe UI"
                    master.Background.Fill.ForeColor.RGB = 0xF0F8FF
                elif theme_name == "Monochrome":
                    master.Background.Fill.ForeColor.RGB = 0x101010
                    master.Shapes.Title.TextFrame.TextRange.Font.Color.RGB = 0xFFFFFF
                elif theme_name == "Quadratic":
                    master.Background.Fill.ForeColor.RGB = 0xFFFFF0
                    master.Shapes.Title.TextFrame.TextRange.Font.Name = "Georgia"

                output_path = os.path.join(templates_folder, template_info['filename'])
                presentation.SaveAs(output_path, 27)
                presentation.Close()

                print(f"✅ Plantilla creada: {template_info['filename']}")

            except Exception as e:
                print(f"❌ Error con {template_info['name']}: {e}")

        ppt_app.Quit()
        print("\n=== Plantillas de PowerPoint creadas! ===")
        print(f"Ubicación: {templates_folder}")

    except Exception as e:
        print(f"Error general: {e}")


if __name__ == "__main__":
    create_custom_templates()
