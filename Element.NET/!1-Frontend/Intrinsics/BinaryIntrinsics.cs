namespace Element.AST
{
    /// <summary>
    /// An intrinsic with 2 arguments and a single return.
    /// </summary>
    internal class BinaryIntrinsic : ICallable
    {
        public BinaryIntrinsic(Binary.Op operation)
        {
            Identifier = new Identifier(operation.ToString().ToLowerInvariant());
            Operation = operation;
        }

        public Identifier Identifier { get; }
        public Binary.Op Operation { get; }

        public Port[] Inputs { get; } =
            {
                // TODO: Add type.
                new Port(new Identifier("a"), null),
                new Port(new Identifier("b"), null),
            };

        public bool CanBeCached => true;

        public IValue Call(CompilationFrame frame, CompilationContext compilationContext) =>
            new Literal(Binary.Evaluate(Operation, 
                                        this.GetArgumentByIndex(0, frame, compilationContext) as Literal, 
                                        this.GetArgumentByIndex(1, frame, compilationContext) as Literal));
    }
}