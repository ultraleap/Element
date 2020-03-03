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
        private DeclaredStruct? _declaredStruct;
        private Port[] Inputs { get; } = {Lexico.Lexico.Parse<Port>("a:Num")};
        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            var argsValid = arguments.ValidateArguments(Inputs, compilationContext.GlobalScope, compilationContext);
            if (!argsValid) return CompilationErr.Instance;
            _declaredStruct ??= compilationContext.GlobalScope[new Identifier(Name), compilationContext] as DeclaredStruct;

            var arg = (Literal)arguments[0];
            return _declaredStruct != null
                ? _declaredStruct.CreateInstance(new IValue[]{new Literal(arg.Value > 0 ? 1f : 0f)}, Instance)
                : compilationContext.LogError(7, $"Couldn't find '{Name}'");
        }
    }
}