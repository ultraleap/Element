namespace Element.AST
{
    /// <summary>
    /// A value that results from a failure during compilation. It will be accepted everywhere and generate no further
    /// errors, returning itself from each operation (the error is non-recoverable).
    /// </summary>
    public sealed class CompilationErr : IValue, IConstraint, IFunction, IType
    {
        public static CompilationErr Instance { get; } = new CompilationErr();
        private CompilationErr() { }
        public override string ToString() => "<error>";
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => false;

        PortInfo[] IFunction.Inputs => null;
        PortInfo[] IFunction.Outputs => null;
        IFunction IFunction.CallInternal(IFunction[] arguments, string output, CompilationContext context) => this;
        public IType Type => Instance;
    }
}