namespace Element.AST
{
    public abstract class Intrinsic
    {
        public abstract string Location { get; }
        public DeclaredItem Declarer { get; set; }
    }
}