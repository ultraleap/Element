namespace Element
{
    public class SourceInfo
    {
        public SourceInfo(string name, string text)
        {
            Name = name;
            OriginalText = text;
            PreprocessedText = Parser.Preprocess(text);
        }
        
        public readonly string Name;
        public readonly string OriginalText;
        public readonly string PreprocessedText;
        
        public (int Line, int Column, int LineCharacterIndex) CountLinesAndColumns(int index, bool indexInPreprocessedText = true)
        {
            var text = indexInPreprocessedText ? PreprocessedText : OriginalText;
            
            var line = 1;
            var column = 1;
            var lineCharacterIndex = 0;
            for (var i = 0; i < index; i++)
            {
                if (text[i] == '\r')
                {
                    line++;
                    column = 1;
                    lineCharacterIndex = 0;
                    if (i + 1 < text.Length && text[i + 1] == '\n')
                    {
                        i++;
                    }
                }
                else if (text[i] == '\n')
                {
                    line++;
                    column = 1;
                    lineCharacterIndex = 0;
                }
                else if (text[i] == '\t')
                {
                    column += 4;
                    lineCharacterIndex++;
                }
                else
                {
                    column++;
                    lineCharacterIndex++;
                }
            }

            return (line, column, lineCharacterIndex);
        }
    }
}