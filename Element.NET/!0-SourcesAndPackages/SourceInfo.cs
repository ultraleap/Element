using System;
using System.IO;
using Element.AST;

namespace Element
{
    /// <summary>
    /// A single named element source.
    /// Could be a source file, a stream of source text or any arbitrary element source code string.
    /// </summary>
    public class SourceInfo
    {
        public static SourceInfo FromFilePath(string path) => FromFile(new FileInfo(path));
        public static SourceInfo FromFile(FileInfo file) => FromStream(file.FullName, File.OpenRead(file.FullName), file.Name);
        public static SourceInfo FromStream(string sourceName, Stream stream, string? displayName = null)
        {
            using var reader = new StreamReader(stream);
            return new SourceInfo(sourceName, reader.ReadToEnd(), displayName);
        }

        public SourceInfo(string fullName, string text, string? displayName = null)
        {
            FullName = fullName;
            DisplayName = displayName ?? fullName;
            OriginalText = text;
            PreprocessedText = Parser.Preprocess(text);
            var splitByNewLine = PreprocessedText.Split(new []{"\r\n", "\n", "\r"}, 3, StringSplitOptions.RemoveEmptyEntries);
            FirstNonEmptyLine = splitByNewLine.Length switch
            {
                0 => string.Empty,
                1 => splitByNewLine[0],
                _ => $"{splitByNewLine[0]}..."
            };
        }
        
        public readonly string FullName;
        public string DisplayName { get; }
        public readonly string OriginalText;
        public readonly string PreprocessedText;
        public readonly string FirstNonEmptyLine;

        /// <summary>
        /// Count line and column numbers for an index in the source text.
        /// </summary>
        public (int Line, int Column, int LineCharacterIndex) CalculateLineAndColumnFromIndex(int index, bool indexInPreprocessedText = true, int spacesPerTab = 4)
        {
            var text = indexInPreprocessedText ? PreprocessedText : OriginalText;
            
            var line = 1;
            var column = 1;
            var lineCharacterIndex = 1;
            
            for (var i = 0; i < index; i++)
            {
                if (text[i] == '\r')
                {
                    line++;
                    column = 1;
                    lineCharacterIndex = 1;
                    if (i + 1 < text.Length && text[i + 1] == '\n')
                    {
                        i++;
                    }
                }
                else if (text[i] == '\n')
                {
                    line++;
                    column = 1;
                    lineCharacterIndex = 1;
                }
                else if (text[i] == '\t')
                {
                    column += spacesPerTab;
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