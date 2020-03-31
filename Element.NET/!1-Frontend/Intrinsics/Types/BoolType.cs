namespace Element.AST
{
    public sealed class BoolType : IntrinsicType<BoolType>
    {
        public override string Name => "Bool";
        public override ISerializer? Serializer => ExtrinsicStruct.StructSerializerInstance;
        public override Port[] Inputs { get; } = {new Port("a", NumType.Instance)};
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            arguments.ValidateArguments(Inputs, compilationContext.GlobalScope, compilationContext)
                ? this.GetDeclaration<DeclaredStruct>(compilationContext)?.CreateInstance(new IValue[] {RefineToBool((Literal) arguments[0])}, Instance)
                : CompilationErr.Instance;
        private static Literal RefineToBool(Literal input) => new Literal(input > 0f ? 1f : 0f);
    }
}