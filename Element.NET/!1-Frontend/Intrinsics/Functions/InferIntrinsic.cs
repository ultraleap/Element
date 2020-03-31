namespace Element.AST
{
    public class InferIntrinsic : IIntrinsic, IFunction
    {
        public IType Type => FunctionType.Instance;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }
        public Port[] Inputs { get; } = {new Port("function", FunctionType.Instance), new Port("instance", AnyConstraint.Instance)};
        public Port? Output => null;

        public string Location => "infer";
    }
}