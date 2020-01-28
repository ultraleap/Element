using System.IO;

namespace Element
{
    /// <summary>
    /// Represents an instance of function usage.
    /// </summary>
    public readonly struct TraceSite
    {
        public TraceSite(string what, FileInfo sourceFile, int line, int column)
        {
            What = what;
            SourceFile = sourceFile;
            Line = line;
            Column = column;
        }

        public readonly FileInfo SourceFile;
        public readonly int Line;
        public readonly int Column;
        public readonly string What;

        public override string ToString() => $"{What} in {SourceFile.FullName}:{Line},{Column}";
    }
}