using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class IfIntrinsic : IntrinsicFunction
    {
        private IfIntrinsic()
        {
            Identifier = new Identifier("if");
            Inputs = new[]
            {
                new Port("condition", BoolType.Instance),
                new Port("ifTrue", AnyConstraint.Instance),
                new Port("ifFalse", AnyConstraint.Instance)
            };
            Output = Port.ReturnPort(AnyConstraint.Instance);
        }
        
        public static IfIntrinsic Instance { get; } = new IfIntrinsic();
		
        public override Identifier Identifier { get; }
        public override IReadOnlyList<Port> Inputs { get; }
        public override Port Output { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
            // Make a list out of the true and false options
            ListType.Instance.Construct(arguments.Skip(1).ToArray(), context)
                    .Cast<StructInstance>(context)
                    // Get the option lists indexer (field 0)
                    .Bind(optionListInstance => optionListInstance.Field(0, context))
                    .Cast<IFunction>(context)
                    // Call the list indexer to get option 0 if true or option 1 if false.
                    .Bind(optionListIndexer => optionListIndexer.Call(new IValue[] {arguments[0] == Constant.True ? Constant.Zero : Constant.One}, context));
    }
}