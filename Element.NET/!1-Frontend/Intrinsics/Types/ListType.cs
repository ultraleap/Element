namespace Element.AST
{
    public class ListType : IIntrinsic, ICallable, IType
    {
        private ListType() {}
        public static ListType Instance { get; } = new ListType();
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type is ListType;
        public string Name => "List";
        public string Location => Name;
        private DeclaredStruct? _declaredStruct;

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            (_declaredStruct ??= compilationContext.GlobalScope[new Identifier(Name), compilationContext] as DeclaredStruct)
            ?.Call(arguments, compilationContext)
            ?? compilationContext.LogError(7, $"Couldn't find '{Name}'");
    }
}