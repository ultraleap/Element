namespace Element.AST
{
    internal class UnaryIntrinsic : IIntrinsic, ICallable
    {
        public UnaryIntrinsic(Unary.Op operation)
        {
            Location = $"Num.{operation.ToString().ToLowerInvariant()}";
            Operation = operation;
        }

        public string Location { get; }
        public Unary.Op Operation { get; }

        public IValue Call(IValue[] arguments, CompilationContext context) =>
            arguments.ValidateArguments(1, context)
                ? (IValue) new Literal(Unary.Evaluate(Operation, (Literal)arguments[0]))
                : CompilationErr.Instance;

        public IType Type => FunctionType.Instance;
    }
}