Sub FormatSlides()
    Dim slide As Slide
    For Each slide In ActivePresentation.Slides
        slide.Shapes.Title.TextFrame.TextRange.Font.Name = "Arial"
        slide.Shapes.Title.TextFrame.TextRange.Font.Size = 32
        slide.Background.Fill.ForeColor.RGB = RGB(240, 240, 240)
    Next slide
End Sub
