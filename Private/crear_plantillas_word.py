import win32com.client as win32
import os


def create_word_template(template_name, font_name, font_size, cover_style, output_path):
    try:
        word_app = win32.Dispatch("Word.Application")
        word_app.Visible = True

        document = word_app.Documents.Add()

        page_setup = document.PageSetup
        page_setup.TopMargin = 72
        page_setup.BottomMargin = 72
        page_setup.LeftMargin = 72
        page_setup.RightMargin = 72

        paragraph_format = document.Content.ParagraphFormat
        paragraph_format.LineSpacingRule = 1
        paragraph_format.LineSpacing = 12 * 2 * 10

        for paragraph in document.Paragraphs:
            for run in paragraph.Range.Font:
                run.Name = font_name
                run.Size = font_size

        cover_page = document.Range(0, 0)
        cover_page.InsertParagraphBefore()
        cover_paragraph = document.Paragraphs(1)
        cover_paragraph.Range.Text = f"Título del Documento ({cover_style})"
        cover_paragraph.Range.Font.Name = font_name
        cover_paragraph.Range.Font.Size = font_size + 4
        cover_paragraph.Range.Font.Bold = True
        cover_paragraph.Alignment = 1

        document.Paragraphs.Add()
        document.Paragraphs.Add()

        author_paragraph = document.Paragraphs(3)
        author_paragraph.Range.Text = "Nombre del Autor"
        author_paragraph.Alignment = 1

        document.Paragraphs.Add()

        date_paragraph = document.Paragraphs(5)
        date_paragraph.Range.Text = "Fecha"
        date_paragraph.Alignment = 1

        document.Paragraphs.Add()
        document.Paragraphs.Add()

        heading1 = document.Paragraphs.Add()
        heading1.Range.Text = "1. Introducción"
        heading1.Range.Font.Name = font_name
        heading1.Range.Font.Size = font_size + 2
        heading1.Range.Font.Bold = True

        document.Paragraphs.Add()

        body_text = document.Paragraphs.Add()
        body_text.Range.Text = "Este es un texto de ejemplo con formato APA. El documento usa sangría francesa y espaciado doble."
        body_text.Range.Font.Name = font_name
        body_text.Range.Font.Size = font_size
        body_text.ParagraphFormat.FirstLineIndent = 36

        output_path = os.path.abspath(output_path)
        document.SaveAs(output_path, 16)
        document.Close()
        word_app.Quit()

        print(f"Plantilla de Word creada: {output_path}")
        return True
    except Exception as e:
        print(f"Error al crear plantilla {template_name}: {e}")
        return False


def create_apa_templates():
    print("=== Creando plantillas de Word (.DOTX) con formato APA ===\n")

    templates_folder = os.path.abspath("templates/word")
    if not os.path.exists(templates_folder):
        os.makedirs(templates_folder)

    templates = [
        {
            "name": "Arial 12",
            "font": "Arial",
            "size": 12,
            "cover": "Portada Clásica",
            "filename": "plantilla_apa_arial_12.dotx"
        },
        {
            "name": "Times New Roman 12",
            "font": "Times New Roman",
            "size": 12,
            "cover": "Portada Académica",
            "filename": "plantilla_apa_times_12.dotx"
        },
        {
            "name": "Calibri 12",
            "font": "Calibri",
            "size": 12,
            "cover": "Portada Moderna",
            "filename": "plantilla_apa_calibri_12.dotx"
        }
    ]

    for i, template_info in enumerate(templates):
        print(f"Creando plantilla {i+1}: {template_info['name']}...")

        output_path = os.path.join(templates_folder, template_info['filename'])

        success = create_word_template(
            template_info['name'],
            template_info['font'],
            template_info['size'],
            template_info['cover'],
            output_path
        )

        if success:
            print(f"✅ Plantilla creada: {template_info['filename']}")
        else:
            print(f"❌ Error al crear plantilla {template_info['name']}")

    print("\n=== Plantillas de Word creadas! ===")
    print(f"Ubicación: {templates_folder}")


if __name__ == "__main__":
    create_apa_templates()
