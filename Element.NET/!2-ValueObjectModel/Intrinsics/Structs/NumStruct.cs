using System.Collections.Generic;
using ResultNET;

namespace Element.AST
{
    /// <summary>
    /// The type for a single number.
    /// </summary>
    public sealed class NumStruct : IIntrinsicStructImplementation
    {
        private NumStruct() { }
        public static NumStruct Instance { get; } = new NumStruct();

        public Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, Context context) =>
            // If the incoming value is an instruction and is num, do nothing
            // If the incoming value is an instruction and isn't a num, emit a cast to change it to num
            // Else the incoming value is not an instruction, which is an error
            arguments.Count == 1
                ? (arguments[0].InnerIs(out Instruction i), i?.StructImplementation == this) switch
                {
                    (true, true)  => new Result<IValue>(arguments[0]),
                    (true, false) => Cast.Create(i, this),
                    _             => context.Trace(EleMessageCode.ConstraintNotSatisfied, $"Argument '{arguments[0]}' is not convertible to Num")
                }
                : context.Trace(EleMessageCode.ArgumentCountMismatch, $"Expected 1 argument but got {arguments.Count}");
        
        public Result<IValue> DefaultValue(Context _) => Constant.Zero;
        public Result MatchesConstraint(Struct @struct, IValue value, Context context)
        {
            Result Error() => context.Trace(EleMessageCode.ConstraintNotSatisfied, $"Expected {Identifier} instance but got {value}");
            return value.InstanceType(context)
                        .Match((type, _) => type.IsSpecificIntrinsic(Instance) ? Result.Success : Error(), _ => Error());
        }
        public Identifier Identifier { get; } = new Identifier("Num");
    }
}