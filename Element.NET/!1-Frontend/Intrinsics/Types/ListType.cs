namespace Element.AST
{
    public class ListType : IIntrinsic, ICallable, IType
    {
        private ListType() {}
        public static ListType Instance { get; } = new ListType();
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;

        public string Location => "List";
        private DeclaredStruct? _declaredStruct;

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            (_declaredStruct ??= compilationContext.GlobalScope[new Identifier(Location), compilationContext] as DeclaredStruct)
            ?.CreateInstance(arguments, Instance)
            ?? compilationContext.LogError(7, $"Couldn't find '{Location}'");

        public string Name => Location;
    }
}