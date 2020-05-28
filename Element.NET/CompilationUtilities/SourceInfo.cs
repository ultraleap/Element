namespace Element
{
    public readonly struct SourceInfo
    {
        public SourceInfo(string name, string text)
        {
            Name = name;
            Text = text;
        }
        
        public readonly string Name;
        public readonly string Text;
        
        public (int Line, int Column, int LineCharacterIndex) CountLinesAndColumns(int index)
        {
            var line = 1;
            var column = 1;
            var lineCharacterIndex = 0;
            for (var i = 0; i < index; i++)
            {
                if (Text[i] == '\r')
                {
                    line++;
                    column = 1;
                    lineCharacterIndex = 0;
                    if (i + 1 < Text.Length && Text[i + 1] == '\n')
                    {
                        i++;
                    }
                }
                else if (Text[i] == '\n')
                {
                    line++;
                    column = 1;
                    lineCharacterIndex = 0;
                }
                else if (Text[i] == '\t')
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