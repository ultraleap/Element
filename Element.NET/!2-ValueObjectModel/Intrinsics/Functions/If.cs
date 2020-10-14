using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class If : IntrinsicValue, IIntrinsicFunctionImplementation
    {
        private If()
        {
            Identifier = new Identifier("if");
        }
        
        public static If Instance { get; } = new If();
        public override Identifier Identifier { get; }
        public bool IsVariadic => false;

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) =>
            // Make a list out of the true and false options
            // 
            List.Instance.Call(arguments.Skip(1).Reverse().ToArray(), context)
                .Cast<StructInstance>(context)
                // Get the option lists indexer (field 0)
                .Bind(optionListInstance => optionListInstance.Index(ListStruct.IndexerId, context))
                .Accumulate(() => context.RootScope.Lookup(NumStruct.Instance.Identifier, context))
                // ReSharper disable once PossibleUnintendedReferenceComparison
                .Bind(t =>
                {
                    var (optionListIndexer, numStruct) = t;
                    // Change condition to a numerical index for the list
                    return numStruct.Call(new[] {arguments[0]}, context)
                                    .Cast<Instruction>(context)
                                    .Bind(index => optionListIndexer.Call(new [] {index}, context));
                });
    }
}