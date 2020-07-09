using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class IfIntrinsicFunctionImplementation : IntrinsicFunctionImplementation
    {
        private IfIntrinsicFunctionImplementation()
        {
            Identifier = new Identifier("if");
        }
        
        public static IfIntrinsicFunctionImplementation Instance { get; } = new IfIntrinsicFunctionImplementation();
        public override Identifier Identifier { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
            // Make a list out of the true and false options
            ListIntrinsicFunctionImplementation.Instance.Call(arguments.Skip(1).ToArray(), context)
                         .Cast<StructInstance>(context)
                         // Get the option lists indexer (field 0)
                         .Bind(optionListInstance => optionListInstance.Index(ListStructImplementation.IndexerId, context))
                         // Call the list indexer to get option 0 if true or option 1 if false.
                         .Bind(optionListIndexer => optionListIndexer.Call(new IValue[] {arguments[0] == Constant.True ? Constant.Zero : Constant.One}, context));
    }
}