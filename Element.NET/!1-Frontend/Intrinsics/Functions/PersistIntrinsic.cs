namespace Element.AST
{
    public class PersistIntrinsic : IIntrinsic, ICallable
    {
        public IType Type => FunctionType.Instance;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }

        public string Location => "persist";
    }
}