using System;
using System.Collections.Generic;
using System.Globalization;
using Element.AST;
using Lexico;

namespace Element
{
    public class Constant : Instruction, IExpressionChainStart
    {
        // ReSharper disable once UnusedMember.Global - Used by Lexico
        public Constant() {}
        public Constant(float value) => Value = value;
        public Constant(float value, IIntrinsicStructImplementation structImplementationOverride) : base(structImplementationOverride) => Value = value;

        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        [Number(ParserFlags = ParserFlags.IgnoreInTrace)] public float Value { get; private set; }
        public static implicit operator float(Constant l) => l.Value;
        
        public enum Intrinsic
        {
            NaN,
            PositiveInfinity,
            NegativeInfinity,
            True,
            False
        }
        
        // NOTE: These are computed properties deliberately! There should be no static instances of IValue implementations otherwise caching breaks.
        public static Constant True => new Constant(1f,  BoolStruct.Instance);
        public static Constant False => new Constant(0f, BoolStruct.Instance);
        public static Constant BoolNaN => new Constant(float.NaN, BoolStruct.Instance); // TODO: Find another solution for propagating NaNs and remove this
        public static Constant Zero => new Constant(0f);
        public static Constant One => new Constant(1f);
        public static Constant NaN => new Constant(float.NaN);
        public static Constant PositiveInfinity => new Constant(float.PositiveInfinity);
        public static Constant NegativeInfinity => new Constant(float.NegativeInfinity);

        public override Result<Constant> CompileTimeConstant(Context context) => this;
        public override IEnumerable<Instruction> Dependent { get; } = Array.Empty<Instruction>();
        public override string TypeOf => StructImplementation.Identifier.String;
        public override string SummaryString => Value.ToString(CultureInfo.CurrentCulture);
        public override bool Equals(Instruction other) => (other as Constant)?.Value == Value;
        // ReSharper disable once NonReadonlyMemberInGetHashCode
        public override int GetHashCode() => new {Value, Type = StructImplementation}.GetHashCode();
        public Result<IValue> Resolve(ExpressionChain expressionChain, IScope scope, Context context) => context.Aspect?.Literal(expressionChain, scope, this) ?? this;

        public string TraceString => SummaryString;
    }

    public class Cast : Instruction
    {
        public static Instruction Create(Instruction instruction, IIntrinsicStructImplementation targetType) => instruction switch
        {
            Constant c => new Constant(c.Value, targetType),
            _ => new Cast(instruction, targetType)
        };

        public override Result<Constant> CompileTimeConstant(Context context) => Instruction.CompileTimeConstant(context);

        public Instruction Instruction { get; }
        private Cast(Instruction instruction, IIntrinsicStructImplementation targetType) : base(targetType) => Instruction = instruction;

        public override IEnumerable<Instruction> Dependent => new[] {Instruction};
        public override string SummaryString => $"{StructImplementation}({Instruction})";
    }
}