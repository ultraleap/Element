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

        private DeclaredStruct? _declaredStruct;
        public IValue? ResolveInstanceFunction(Identifier id, Literal instanceBeingIndexed, CompilationContext compilationContext)
        {
            if (_declaredStruct == null)
            {
                lock (this)
                {
                    if (!(compilationContext.GlobalScope[new Identifier(Location), false, compilationContext] is DeclaredStruct declaredStruct))
                    {
                        return compilationContext.LogError(7, $"Couldn't find '{Location}'");
                    }

                    _declaredStruct = declaredStruct;
                }
            }

            return _declaredStruct.ResolveInstanceFunction(id, instanceBeingIndexed, compilationContext);
        }
    }
}