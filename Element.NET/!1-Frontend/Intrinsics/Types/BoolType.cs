namespace Element.AST
{
    public sealed class BoolType : IIntrinsic, IFunction, IType
    {
        private BoolType() {}
        public static BoolType Instance { get; } = new BoolType();
        public static TypeAnnotation Annotation { get; } = new TypeAnnotation(Instance);
        public IType Type => TypeType.Instance;
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == Instance;
        public string Name => "Bool";
        public string Location => Name;
        public Port[] Inputs { get; } = {new Port("a", NumType.Annotation)};
        public TypeAnnotation Output => Annotation;
        private DeclaredStruct? _declaredStruct;

        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            if (_declaredStruct == null)
            {
                lock (this)
                {
                    if (_declaredStruct == null)
                    {
                        if (!(compilationContext.GlobalScope[new Identifier(Location), false, compilationContext] is
                            DeclaredStruct declaredStruct))
                        {
                            return compilationContext.LogError(7, $"Couldn't find '{Location}'");
                        }

                        _declaredStruct = declaredStruct;
                    }
                }
            }

            return arguments.ValidateArguments(Inputs, compilationContext.GlobalScope, compilationContext)
                ? _declaredStruct.CreateInstance(new IValue[] {new Literal((Literal) arguments[0] > 0f ? 1f : 0f)}, Instance)
                : CompilationErr.Instance;
        }
    }
}