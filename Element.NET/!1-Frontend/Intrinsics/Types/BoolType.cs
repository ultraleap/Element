namespace Element.AST
{
    public class BoolType : IIntrinsic, ICallable, IType
    {
        private BoolType() {}
        public static BoolType Instance { get; } = new BoolType();
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
        public string Name => "Bool";
        public string Location => Name;
        private Port[] Inputs { get; } = {new Port("a", NumType.Instance)};

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            arguments.ValidateArguments(Inputs, compilationContext.GlobalScope, compilationContext)
                ? (IValue)new Literal((Literal)arguments[0], Instance)
                : CompilationErr.Instance;
    }
}