namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IIntrinsic, ICallable, IType
    {
        private NumType() { }
        public static NumType Instance { get; } = new NumType();
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
        public string Name => "Num";
        public string Location => Name;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            arguments.ValidateArguments(1, compilationContext)
                ? arguments[0] is Literal lit
                      ? (IValue) lit
                      : compilationContext.LogError(8, "Argument must be a number")
                : CompilationErr.Instance;

    }
}