namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IIntrinsic, IFunction, IType
    {
        private NumType() { }
        public static NumType Instance { get; } = new NumType();
        public static TypeAnnotation Annotation { get; } = new TypeAnnotation(Instance);

        public Port[] Inputs { get; } = {new Port("a", Annotation)};
        public TypeAnnotation Output => Annotation;
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

        public IValue? ResolveInstanceFunction(Identifier id, Literal instanceBeingIndexed, CompilationContext compilationContext) =>
            compilationContext.GlobalScope[new Identifier(Location), false, compilationContext] switch
            {
                DeclaredStruct declaredStruct => declaredStruct.ResolveInstanceFunction(id, instanceBeingIndexed, compilationContext),
                _ => compilationContext.LogError(7, $"Couldn't find '{Location}'")
            };
    }
}