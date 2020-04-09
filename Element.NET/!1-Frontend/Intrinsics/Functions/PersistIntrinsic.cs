namespace Element.AST
{
    public class PersistIntrinsic : IntrinsicFunction
    {
        public PersistIntrinsic()
            : base("persist", new[]
            {
                new Port("initial", AnyConstraint.Instance),
                new Port("body", FunctionType.Instance)
            }, Port.ReturnPort(AnyConstraint.Instance))
        { }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }
    }
}