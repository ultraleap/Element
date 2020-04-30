using System.Linq;

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
            if (!(ListType.Instance.MakeList(arguments.Skip(1).ToArray(), compilationContext) is StructInstance optionsList))
            {
                return compilationContext.LogError(14, "Couldn't construct 'if' options list");
            }

            // Use index 0 if true or index 1 if false.
            var index = arguments[0] == Constant.True ? Constant.Zero : Constant.One;
            return (optionsList[0] as IFunctionSignature).ResolveCall(new IValue[] {index}, false, compilationContext);
        }
    }
}