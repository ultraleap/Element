namespace Element.AST
{
    public sealed class IfIntrinsic : IntrinsicFunction
    {
        public IfIntrinsic()
            : base("Bool.if",
                   new[]
                   {
                       new Port("condition", BoolType.Instance),
                       new Port("ifTrue", AnyConstraint.Instance),
                       new Port("ifFalse", AnyConstraint.Instance)
                   }, Port.ReturnPort(AnyConstraint.Instance))
        { }
		
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            // NOTE: Swaps arguments 2 and 1 positions when making list so that they are at index 0 and 1 corresponding to false/true
            if (!(ListType.Instance.MakeList(new[]{arguments[2], arguments[1]}, compilationContext) is StructInstance optionsList))
            {
                return compilationContext.LogError(14, "Couldn't construct 'if' options list");
            }
            
            return (optionsList[0] as IFunctionSignature).ResolveCall(new[] {arguments[0]}, false, compilationContext);
        }
    }
}