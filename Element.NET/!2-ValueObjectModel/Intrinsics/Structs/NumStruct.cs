using System.Collections.Generic;

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
            // If the incoming value isn't a num, emit a cast to change it to num
            // Else do nothing - the argument will have been type checked already by Call before getting to here
            arguments[0].IsType(out Instruction i) && i.StructImplementation != this
                ? Cast.Create(i, this)
                : new Result<IValue>(arguments[0]);
        public Result<IValue> DefaultValue(Context _) => Constant.Zero;
        public Result<bool> MatchesConstraint(Struct @struct, IValue value, Context context) => value.IsType<Instruction>(out var instruction) && instruction.StructImplementation == Instance;
        public Identifier Identifier { get; } = new Identifier("Num");
    }
}