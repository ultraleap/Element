using System.Collections.Generic;

namespace Element.AST
{
    public sealed class BoolStruct : IIntrinsicStructImplementation
    {
        private BoolStruct() { }
        public static BoolStruct Instance { get; } = new BoolStruct();
        public Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, Context context) =>
            arguments.Count == 1
                ? Element.Binary.CreateAndOptimize(Element.Binary.Op.Gt, arguments[0], Constant.Zero, context)
                         .Bind(condition => If.Instance.Call(new[]{condition, Constant.True, Constant.False}, context))
                          : context.Trace(EleMessageCode.ArgumentCountMismatch, $"Expected 1 argument but got {arguments.Count}");
        public Result<IValue> DefaultValue(Context _) => Constant.False;

        public Result MatchesConstraint(Struct @struct, IValue value, Context context)
        {
            Result Error() => context.Trace(EleMessageCode.ConstraintNotSatisfied, $"Expected {Identifier} instance but got {value}");
            return value.InstanceType(context)
                        .Match((type, _) => type.IsSpecificIntrinsic(Instance) ? Result.Success : Error(), _ => Error());
        }

        public Identifier Identifier { get; } = new Identifier("Bool");
    }
}