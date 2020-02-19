namespace Element.AST
{
    /// <summary>
    /// An intrinsic with 2 arguments and a single return.
    /// </summary>
    internal class BinaryIntrinsic : ICallable
    {
        public BinaryIntrinsic(Binary.Op operation)
        {
            FullPath = $"Num.{operation.ToString().ToLowerInvariant()}";
            Operation = operation;
        }

        public string FullPath { get; }
        public Binary.Op Operation { get; }

        public Port[] Inputs { get; } =
            {
                // TODO: Add type.
                new Port(new Identifier("a"), null),
                new Port(new Identifier("b"), null),
            };

        public bool CanBeCached => true;

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            new Literal(Binary.Evaluate(Operation, arguments[0] as Literal, arguments[1] as Literal));
    }
}