import os
import win32com.client as win32
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from docx import Document
from docx.shared import Inches as DocxInches, Pt as DocxPt, RGBColor as DocxRGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH


class PowerPointAgent:
    def __init__(self, template_path=None):
        self.template_path = template_path
        self.presentation = None
        self.color_themes = {
            "professional": {
                "background": RGBColor(240, 245, 250),
                "accent": RGBColor(0, 102, 204),
                "text": RGBColor(30, 30, 30)
            },
            "modern": {
                "background": RGBColor(20, 20, 30),
                "accent": RGBColor(0, 255, 200),
                "text": RGBColor(240, 240, 240)
            },
            "vibrant": {
                "background": RGBColor(255, 248, 220),
                "accent": RGBColor(255, 87, 51),
                "text": RGBColor(30, 30, 30)
            }
        }

    def _convert_potx_to_pptx(self, potx_path):
        potx_path = os.path.abspath(potx_path)
        temp_pptx = potx_path.replace(".potx", "_temp.pptx")
        try:
            ppt_app = win32.Dispatch("PowerPoint.Application")
            ppt_app.Visible = True
            pres = ppt_app.Presentations.Open(potx_path)
            pres.SaveAs(temp_pptx, 11)
            pres.Close()
            ppt_app.Quit()
            return temp_pptx
        except Exception as e:
            print(f"Error al convertir POTX: {e}")
            return None

    def create_presentation(self, output_path, slides_data, theme="professional"):
        temp_file = None
        if self.template_path and os.path.exists(self.template_path):
            template_path = self.template_path
            if template_path.endswith(".potx"):
                temp_file = self._convert_potx_to_pptx(template_path)
                if temp_file:
                    self.presentation = Presentation(temp_file)
                else:
                    self.presentation = Presentation()
            else:
                self.presentation = Presentation(template_path)
        else:
            self.presentation = Presentation()

        colors = self.color_themes.get(theme, self.color_themes["professional"])

        for slide_data in slides_data:
            slide_layout = self.presentation.slide_layouts[slide_data.get("layout", 0)]
            slide = self.presentation.slides.add_slide(slide_layout)
            
            if slide_data.get("background"):
                self.set_background_color(slide, colors["background"])
            
            for shape in slide.shapes:
                if hasattr(shape, "text"):
                    shape.text = slide_data.get("text", "")
                    self.format_shape_text(shape, colors)
            
            if "title" in slide_data and slide.shapes.title:
                slide.shapes.title.text = slide_data["title"]
                self.format_title(slide.shapes.title, colors)
            
            if "subtitle" in slide_data and len(slide.placeholders) > 1:
                slide.placeholders[1].text = slide_data["subtitle"]

        output_path = os.path.abspath(output_path)
        self.presentation.save(output_path)
        
        if temp_file and os.path.exists(temp_file):
            try:
                os.remove(temp_file)
            except:
                pass
                
        return output_path

    def set_background_color(self, slide, rgb_color):
        background = slide.background
        fill = background.fill
        fill.solid()
        fill.fore_color.rgb = rgb_color

    def format_shape_text(self, shape, colors):
        for paragraph in shape.text_frame.paragraphs:
            for run in paragraph.runs:
                run.font.size = Pt(18)
                run.font.color.rgb = colors["text"]

    def format_title(self, shape, colors):
        for paragraph in shape.text_frame.paragraphs:
            for run in paragraph.runs:
                run.font.size = Pt(36)
                run.font.bold = True
                run.font.color.rgb = colors["accent"]

    def add_vba_macro(self, ppt_path, vba_code):
        try:
            ppt_path = os.path.abspath(ppt_path)
            ppt_app = win32.Dispatch("PowerPoint.Application")
            ppt_app.Visible = True
            presentation = ppt_app.Presentations.Open(ppt_path)
            
            vb_project = presentation.VBProject
            vb_component = vb_project.VBComponents.Add(1)
            vb_component.CodeModule.AddFromString(vba_code)
            
            presentation.Save()
            presentation.Close()
            ppt_app.Quit()
        except Exception as e:
            print(f"Error al agregar macro a PowerPoint: {e}")
            print("Asegúrate de habilitar el acceso al modelo de objetos VBA en las opciones de PowerPoint.")


class WordAgent:
    def __init__(self, template_path=None):
        self.template_path = template_path
        self.document = None
        self.styles = {
            "professional": {
                "heading_font": "Arial",
                "body_font": "Calibri",
                "heading_color": DocxRGBColor(0, 102, 204),
                "body_color": DocxRGBColor(30, 30, 30)
            },
            "modern": {
                "heading_font": "Segoe UI",
                "body_font": "Segoe UI",
                "heading_color": DocxRGBColor(0, 200, 150),
                "body_color": DocxRGBColor(50, 50, 50)
            }
        }

    def _convert_dotx_to_docx(self, dotx_path):
        dotx_path = os.path.abspath(dotx_path)
        temp_docx = dotx_path.replace(".dotx", "_temp.docx")
        try:
            word_app = win32.Dispatch("Word.Application")
            word_app.Visible = True
            doc = word_app.Documents.Open(dotx_path)
            doc.SaveAs(temp_docx, 16)
            doc.Close()
            word_app.Quit()
            return temp_docx
        except Exception as e:
            print(f"Error al convertir DOTX: {e}")
            return None

    def create_document(self, output_path, content_data, style="professional"):
        temp_file = None
        if self.template_path and os.path.exists(self.template_path):
            template_path = self.template_path
            if template_path.endswith(".dotx"):
                temp_file = self._convert_dotx_to_docx(template_path)
                if temp_file:
                    self.document = Document(temp_file)
                else:
                    self.document = Document()
            else:
                self.document = Document(template_path)
        else:
            self.document = Document()

        doc_style = self.styles.get(style, self.styles["professional"])

        for content in content_data:
            if content["type"] == "heading":
                self.add_heading(content, doc_style)
            elif content["type"] == "paragraph":
                self.add_paragraph(content, doc_style)
            elif content["type"] == "table":
                self.add_table(content, doc_style)
            elif content["type"] == "image":
                self.add_image(content)

        output_path = os.path.abspath(output_path)
        self.document.save(output_path)
        
        if temp_file and os.path.exists(temp_file):
            try:
                os.remove(temp_file)
            except:
                pass
                
        return output_path

    def add_heading(self, content, style):
        level = content.get("level", 1)
        heading = self.document.add_heading(content["text"], level=level)
        
        for run in heading.runs:
            run.font.name = style["heading_font"]
            run.font.color.rgb = style["heading_color"]
            run.font.bold = True
            if level == 1:
                run.font.size = DocxPt(24)
            elif level == 2:
                run.font.size = DocxPt(18)
            else:
                run.font.size = DocxPt(14)

    def add_paragraph(self, content, style):
        paragraph = self.document.add_paragraph()
        paragraph.alignment = content.get("alignment", WD_ALIGN_PARAGRAPH.JUSTIFY)
        run = paragraph.add_run(content["text"])
        run.font.name = style["body_font"]
        run.font.size = DocxPt(11)
        run.font.color.rgb = style["body_color"]

    def add_table(self, content, style):
        data = content["data"]
        table = self.document.add_table(rows=len(data), cols=len(data[0]))
        table.style = 'Table Grid'
        
        for i, row in enumerate(data):
            for j, cell_text in enumerate(row):
                cell = table.rows[i].cells[j]
                cell.text = cell_text
                for paragraph in cell.paragraphs:
                    for run in paragraph.runs:
                        run.font.name = style["body_font"]
                        if i == 0:
                            run.font.bold = True
                            run.font.color.rgb = style["heading_color"]

    def add_image(self, content):
        if os.path.exists(content["path"]):
            self.document.add_picture(
                content["path"],
                width=DocxInches(content.get("width", 6))
            )

    def add_vba_macro(self, doc_path, vba_code):
        try:
            doc_path = os.path.abspath(doc_path)
            word_app = win32.Dispatch("Word.Application")
            word_app.Visible = True
            document = word_app.Documents.Open(doc_path)
            
            vb_project = document.VBProject
            vb_component = vb_project.VBComponents.Add(1)
            vb_component.CodeModule.AddFromString(vba_code)
            
            document.Save()
            document.Close()
            word_app.Quit()
        except Exception as e:
            print(f"Error al agregar macro a Word: {e}")
            print("Asegúrate de habilitar el acceso al modelo de objetos VBA en las opciones de Word.")


class OfficeAgent:
    def __init__(self):
        self.powerpoint_agent = PowerPointAgent()
        self.word_agent = WordAgent()

    def create_powerpoint(self, output_path, slides_data, template_path=None, vba_code=None, theme="professional"):
        self.powerpoint_agent.template_path = template_path
        ppt_path = self.powerpoint_agent.create_presentation(output_path, slides_data, theme)
        
        if vba_code:
            self.powerpoint_agent.add_vba_macro(ppt_path, vba_code)
        
        return ppt_path

    def create_word_document(self, output_path, content_data, template_path=None, vba_code=None, style="professional"):
        self.word_agent.template_path = template_path
        doc_path = self.word_agent.create_document(output_path, content_data, style)
        
        if vba_code:
            self.word_agent.add_vba_macro(doc_path, vba_code)
        
        return doc_path
