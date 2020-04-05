namespace Element.AST
{
    public class MemberwiseIntrinsic : IntrinsicFunction
    {
        public MemberwiseIntrinsic()
            : base("memberwise",
                   new[]
                   {
                       new Port("function", FunctionType.Instance),
                       Port.VariadicPort
                   }, Port.ReturnPort(AnyConstraint.Instance))
        { }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }
    }
}