namespace Element.AST
{
    /// <summary>
    /// A value that results from a failure during compilation. It will be accepted everywhere and generate no further
    /// errors, returning itself from each operation (the error is non-recoverable).
    /// </summary>
    public sealed class CompilationErr : IFunction, IType, Element.IFunction
    {
        public static CompilationErr Instance { get; } = new CompilationErr();
        private CompilationErr() { }
        public override string ToString() => "<error>";
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => false;
        Port[] IFunction.Inputs => null;
        Type IFunction.Output => null;
        IValue ICallable.Call(IValue[] arguments, CompilationContext compilationContext) => this;
        IType IValue.Type => Instance;

        // TODO: Delete these
        PortInfo[] Element.IFunction.Inputs { get; } = null;
        PortInfo[] Element.IFunction.Outputs { get; } = null;
        public Element.IFunction CallInternal(Element.IFunction[] arguments, string output, CompilationContext context) => this;
        public string Name { get; } = "<error>";
    }
}