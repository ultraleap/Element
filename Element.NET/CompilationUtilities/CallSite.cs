namespace Element
{
    /// <summary>
    /// Represents an instance of function usage.
    /// </summary>
    public readonly struct CallSite
    {
        public CallSite(IFunction function, string sourceFile, int line, int column)
        {
            Function = function;
            SourceFile = sourceFile;
            Line = line;
            Column = column;
        }
            
        public readonly string SourceFile;
        public readonly int Line;
        public readonly int Column;
        public readonly IFunction Function;

        public override string ToString() => $"<{SourceFile}|{Line}:{Column}> {Function}";
    }
}