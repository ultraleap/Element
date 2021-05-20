using ResultNET;

namespace Element.AST
{
    /// <summary>
    /// Lexical scope which can perform lookup.
    /// </summary>
    public interface IScope
    {
        public abstract Result<IValue> Lookup(Identifier id, Context context);
    }
}