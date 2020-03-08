namespace Element.AST
{
    public class InferIntrinsic : IIntrinsic, ICallable
    {
        public IType Type => FunctionType.Instance;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }

        public string Location => "infer";
    }
}