namespace Element.AST
{
    /// <summary>
    /// An intrinsic with 2 arguments and a single return.
    /// </summary>
    internal class BinaryIntrinsic : IIntrinsic, IFunction
    {
        public BinaryIntrinsic(Binary.Op operation)
        {
            Location = $"Num.{operation.ToString().ToLowerInvariant()}";
            Operation = operation;
        }

        public string Location { get; }
        public Binary.Op Operation { get; }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            arguments.ValidateArguments(2, compilationContext)
                ? (IValue) new Literal(Binary.Evaluate(Operation, (Literal)arguments[0], (Literal)arguments[1]))
                : CompilationErr.Instance;

        public IType Type => FunctionType.Instance;
        public Port[] Inputs { get; } = {new Port("a", NumType.Instance), new Port("b", NumType.Instance)};
        public Type Output { get; } = new Type(NumType.Instance);
    }
}