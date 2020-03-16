using Element.AST;

namespace Element
{
    /// <summary>
    /// Represents an instance of function usage.
    /// </summary>
    public readonly struct TraceSite
    {
        public TraceSite(IValue what, string source, int line, int column)
        {
            What = what;
            Source = source;
            Line = line;
            Column = column;
        }

        public readonly IValue What;
        public readonly string Source;
        public readonly int Line;
        public readonly int Column;

        public override string ToString() => $"{What} in {Source}:{Line},{Column}";
    }
}