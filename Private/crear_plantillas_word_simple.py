import win32com.client as win32
import os


def create_word_template_simple():
    print("=== Creando plantillas de Word (.DOTX) con formato APA ===\n")

    templates_folder = os.path.abspath("templates/word")
    if not os.path.exists(templates_folder):
        os.makedirs(templates_folder)

    templates = [
        {
            "font": "Arial",
            "size": 12,
            "filename": "plantilla_apa_arial_12.dotx"
        },
        {
            "font": "Times New Roman",
            "size": 12,
            "filename": "plantilla_apa_times_12.dotx"
        },
        {
            "font": "Calibri",
            "size": 12,
            "filename": "plantilla_apa_calibri_12.dotx"
        }
    ]

    try:
        word_app = win32.Dispatch("Word.Application")
        word_app.Visible = True

        for i, template_info in enumerate(templates):
            try:
                print(f"Creando plantilla {i+1}: {template_info['font']} {template_info['size']}...")

                doc = word_app.Documents.Add()

                style = doc.Styles("Normal")
                style.Font.Name = template_info["font"]
                style.Font.Size = template_info["size"]
                style.ParagraphFormat.LineSpacingRule = 1
                style.ParagraphFormat.LineSpacing = 12 * 2 * 10
                style.ParagraphFormat.FirstLineIndent = 36

                page_setup = doc.PageSetup
                page_setup.TopMargin = 72
                page_setup.BottomMargin = 72
                page_setup.LeftMargin = 72
                page_setup.RightMargin = 72

                output_path = os.path.join(templates_folder, template_info["filename"])
                doc.SaveAs(output_path, 16)
                doc.Close()

                print(f"✅ Plantilla creada: {template_info['filename']}")

            except Exception as e:
                print(f"❌ Error: {e}")

        word_app.Quit()
        print("\n=== Plantillas de Word creadas! ===")
        print(f"Ubicación: {templates_folder}")

    except Exception as e:
        print(f"Error general: {e}")


if __name__ == "__main__":
    create_word_template_simple()
