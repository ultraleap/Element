namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumType : IIntrinsicType
    {
        private NumType()
        {
            Inputs = new[]{new Port("a", Instance)};
            Output = Port.ReturnPort(Instance);
        }
        public static NumType Instance { get; } = new NumType();
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value is Element.Expression expr && expr.Type == Instance;
        public Port[] Inputs { get; }
        public Port Output { get; }
        public IValue Call(IValue[] arguments, CompilationContext compilationContext) => arguments[0]; // Return the number - type checking will already have occurred.
        string IIntrinsic.Location => "Num";
        public ISerializableValue DefaultValue(CompilationContext _) => Constant.Zero;
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this.GetDeclaration(compilationContext);

    }
}