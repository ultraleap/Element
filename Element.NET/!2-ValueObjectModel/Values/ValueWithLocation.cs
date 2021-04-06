using System.Linq;

namespace Element.AST
{
    /// <summary>
    /// Value retaining it's full path in source and identifiers within, including it's own identifier.
    /// </summary>
    public class ValueWithLocation : WrapperValue, ISourceLocation
    {
        public ValueWithLocation(Identifier[] identifiersInPath, IValue value, int indexInSource, SourceInfo sourceInfo) : base(value)
        {
            Identifier = identifiersInPath.Last();
            IdentifiersInPath = identifiersInPath;
            IndexInSource = indexInSource;
            SourceInfo = sourceInfo;
            FullPath = string.Join(".", IdentifiersInPath);
        }

        public Identifier Identifier { get; }
        public Identifier[] IdentifiersInPath { get; }
        public string FullPath { get; }
        public int IndexInSource { get; }
        public SourceInfo SourceInfo { get; }
    }
}