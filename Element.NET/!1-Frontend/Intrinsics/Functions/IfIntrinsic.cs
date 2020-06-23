using System.Linq;

namespace Element.AST
{
    public sealed class IfIntrinsic : IIntrinsicFunction
    {
        private IfIntrinsic()
        {
            Inputs = new[]
            {
                new Port("condition", BoolType.Instance),
                new Port("ifTrue", AnyConstraint.Instance),
                new Port("ifFalse", AnyConstraint.Instance)
            };
            Output = Port.ReturnPort(AnyConstraint.Instance);
        }
        
        public static IfIntrinsic Instance { get; } = new IfIntrinsic();
		
        public Result<IValue> Call(IValue[] arguments)
        {
            if (!(ListType.Instance.MakeList(arguments.Skip(1).ToArray()) is StructInstance optionsList))
            {
                return compilationContext.Trace(MessageCode.TypeError, "Couldn't construct 'if' options list");
            }

            // Use index 0 if true or index 1 if false.
            var index = arguments[0] == Constant.True ? Constant.Zero : Constant.One;
            return ((IFunctionSignature) optionsList[0]).ResolveCall(new IValue[] {index}, false, compilationContext);
        }

        public Port[] Inputs { get; }
        public Port Output { get; }
    }
}