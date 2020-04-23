namespace Element.AST
{
    /// <summary>
    /// An intrinsic with 2 arguments and a single return.
    /// </summary>
    public sealed class BinaryIntrinsic : IntrinsicFunction
    {
        public BinaryIntrinsic(Binary.Op operation)
            : base($"Num.{operation.ToString().ToLowerInvariant()}",
                   new[]
                   {
                       new Port("a", NumType.Instance),
                       new Port("b", NumType.Instance)
                   },
                   Port.ReturnPort(NumType.Instance)) =>
            Operation = operation;

        public Binary.Op Operation { get; }

        public override IValue Call(IValue[] arguments, CompilationContext _) =>
            new Binary(Operation, arguments[0] as Element.Expression, arguments[1] as Element.Expression);
    }
}