namespace ResultNET
{
    /// <summary>
    /// A trace at a location in source.
    /// </summary>
    public readonly struct TraceSite
    {
        public TraceSite(string what, string source, int line, int column)
        {
            What = what;
            Source = source;
            Line = line;
            Column = column;
            _string = $"{What} at {Source}:{Line},{Column}";
        }

        public readonly string What;
        public readonly string Source;
        public readonly int Line;
        public readonly int Column;

        private readonly string _string;
        public override string ToString() => _string;
    }
}