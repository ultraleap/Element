namespace Element.AST
{
    internal class UnaryIntrinsic : IntrinsicFunction
    {
        public UnaryIntrinsic(Unary.Op operation)
            : base($"Num.{operation.ToString().ToLowerInvariant()}",
                new[]
                {
                    new Port("a", NumType.Instance)
                }, Port.ReturnPort(NumType.Instance)) =>
            Operation = operation;
        
        public Unary.Op Operation { get; }

        public override IValue Call(IValue[] arguments, CompilationContext _) => new Unary(Operation, arguments[0] as Element.Expression);
    }
}