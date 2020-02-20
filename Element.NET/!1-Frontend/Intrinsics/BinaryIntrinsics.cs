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

        public bool CanBeCached => true;

        // TODO: Add type checking
        public IValue Call(IValue[] arguments, CompilationContext compilationContext) => arguments.ValidateArgumentCount(2, compilationContext)
            ? (IValue) new Literal(Binary.Evaluate(Operation, arguments[0] as Literal, arguments[1] as Literal))
            : CompilationErr.Instance;
    }
}