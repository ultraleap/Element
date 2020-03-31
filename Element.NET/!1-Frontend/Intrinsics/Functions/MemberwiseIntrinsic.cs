namespace Element.AST
{
    public class MemberwiseIntrinsic : IIntrinsic, IFunction
    {
        public IType Type => FunctionType.Instance;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }
        public Port[] Inputs { get; } = {new Port("function", FunctionType.Instance), Port.VariadicPort};
        public Port? Output => null;

        public string Location => "memberwise";
    }
}