namespace Element.AST
{
    public class InferIntrinsic : IntrinsicFunction
    {
        public InferIntrinsic()
            : base("infer",
                   new[]
                   {
                       new Port("function", FunctionType.Instance),
                       new Port("instance", AnyConstraint.Instance)
                   }, Port.ReturnPort(AnyConstraint.Instance))
        { }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }
    }
}