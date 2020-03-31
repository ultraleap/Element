namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IntrinsicType<NumType>
    {
        public override Port[] Inputs { get; } = {new Port("a", Instance)};
        public override string Name => "Num";
        public override ISerializer? Serializer { get; } = new NumSerializer();

        private class NumSerializer : ISerializer
        {
            int ISerializer.SerializedSize(IValue _) => 1;

            public bool Serialize(IValue value, ref float[] array, ref int position)
            {
                if (!(value is Literal lit)) return false;
                array[position] = lit;
                position++;
                return true;
            }
        }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            arguments.ValidateArguments(1, compilationContext)
                ? arguments[0] is Literal lit
                      ? (IValue) lit
                      : compilationContext.LogError(8, "Argument must be a number")
                : CompilationErr.Instance;

        public IValue? ResolveInstanceFunction(Identifier id, Literal instanceBeingIndexed, CompilationContext compilationContext) =>
            this.GetDeclaration<DeclaredStruct>(compilationContext)?.ResolveInstanceFunction(id, instanceBeingIndexed, compilationContext);
    }
}