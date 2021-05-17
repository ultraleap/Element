using System.Collections.Generic;
using System.Linq;
using ResultNET;

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
            // Make a list out of the true and false flags
            List.Instance.Call(arguments.Skip(1).Reverse().ToArray(), context)
                .CastInner<StructInstance>()
                // Get the option lists indexer (field 0)
                .Bind(optionListInstance => optionListInstance.Index(ListStruct.IndexerId, context))
                .Accumulate(() => context.RootScope.Lookup(NumStruct.Instance.Identifier, context))
                // ReSharper disable once PossibleUnintendedReferenceComparison
                .Bind(t =>
                          // Change condition to a numerical index for the list
                          t.Item2.Call(new[] {arguments[0]}, context)
                           .CastInner<Instruction>()
                           .Bind(index => t.Item1.Call(new[] {index}, context)));
    }
}
