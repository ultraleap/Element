namespace Element.AST
{
    public sealed class ListType : IIntrinsic, IFunction, IType
    {
        private ListType() {}
        public static ListType Instance { get; } = new ListType();
        public static TypeAnnotation Annotation { get; } = new TypeAnnotation(Instance);
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;

        public string Location => "List";
        public string Name => Location;
        private DeclaredStruct? _declaredStruct;

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            (_declaredStruct ??= compilationContext.GlobalScope[new Identifier(Location), false, compilationContext] as DeclaredStruct)
            ?.CreateInstance(arguments, Instance)
            ?? compilationContext.LogError(7, $"Couldn't find '{Location}'");

        public Port[] Inputs { get; } = {new Port("at", FunctionType.Annotation), new Port("count", NumType.Annotation)};
        public TypeAnnotation Output => Annotation;
    }
}