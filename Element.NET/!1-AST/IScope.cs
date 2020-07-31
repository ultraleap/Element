namespace Element.AST
{
    public interface IScope
    {
        public abstract Result<IValue> Lookup(Identifier id, Context context);
    }
}