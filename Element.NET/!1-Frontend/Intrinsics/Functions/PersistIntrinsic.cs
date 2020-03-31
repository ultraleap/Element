namespace Element.AST
{
    public class PersistIntrinsic : IIntrinsic, IFunction
    {
        public IType Type => FunctionType.Instance;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }
        public Port[] Inputs { get; } = {new Port("initial", AnyConstraint.Instance), new Port("body", FunctionType.Instance)};
        public Port? Output => null;

        public string Location => "persist";
    }
}