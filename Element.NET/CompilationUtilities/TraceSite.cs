using System.IO;
using Element.AST;

namespace Element
{
    /// <summary>
    /// Represents an instance of function usage.
    /// </summary>
    public readonly struct TraceSite
    {
        public TraceSite(IValue what, FileInfo sourceFile, int line, int column)
        {
            What = what;
            SourceFile = sourceFile;
            Line = line;
            Column = column;
        }

        public readonly FileInfo SourceFile;
        public readonly int Line;
        public readonly int Column;
        public readonly IValue What;

        public override string ToString() => $"{What} in {SourceFile?.FullName}:{Line},{Column}";
    }
}