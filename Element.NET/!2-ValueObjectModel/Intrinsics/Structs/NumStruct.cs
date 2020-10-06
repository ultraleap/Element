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
        public Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, Context context) => arguments[0] switch
        {
            Instruction i when i.StructImplementation != this => Cast.Create(i, this),
            {} num => new Result<IValue>(num)
        };
        public Result<IValue> DefaultValue(Context _) => Constant.Zero;
        public Result<bool> MatchesConstraint(Struct @struct, IValue value, Context context) => value is Instruction expr && expr.StructImplementation == Instance;
        public Identifier Identifier { get; } = new Identifier("Num");
    }
}