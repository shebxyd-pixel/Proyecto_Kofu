Sub FormatDocument()
    With ActiveDocument.Content.Font
        .Name = "Calibri"
        .Size = 11
    End With
    ActiveDocument.Paragraphs.SpaceAfter = 6
    ActiveDocument.PageSetup.LeftMargin = 72
    ActiveDocument.PageSetup.RightMargin = 72
End Sub
