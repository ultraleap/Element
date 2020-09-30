namespace Element
{
    /// <summary>
    /// Represents an instance of function usage.
    /// </summary>
    public readonly struct TraceSite
    {
        public TraceSite(string what, string source, int line, int column)
        {
            What = what;
            Source = source;
            Line = line;
            Column = column;
        }

        public readonly string What;
        public readonly string Source;
        public readonly int Line;
        public readonly int Column;

        public override string ToString() => $"{What} at {Source}:{Line},{Column}";
    }
}