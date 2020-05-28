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
            if (!(ListType.Instance.MakeList(arguments.Skip(1).ToArray(), compilationContext) is StructInstance optionsList))
            {
                return compilationContext.LogError(14, "Couldn't construct 'if' options list");
            }

            // Use index 0 if true or index 1 if false.
            var index = arguments[0] == Constant.True ? Constant.Zero : Constant.One;
            return ((IFunctionSignature) optionsList[0]).ResolveCall(new IValue[] {index}, false, compilationContext);
        }
    }
}