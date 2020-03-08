namespace Element.AST
{
    public class MemberwiseIntrinsic : IIntrinsic, ICallable
    {
        public IType Type => FunctionType.Instance;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }

        public string Location => "memberwise";
    }
}