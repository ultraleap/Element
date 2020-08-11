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
        private Constant(float value, IIntrinsicStructImplementation structImplementationOverride) : base(structImplementationOverride) => Value = value;

        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        [Term] public float Value { get; private set; }
        public static implicit operator float(Constant l) => l.Value;
        
        public enum Intrinsic
        {
            NaN,
            PositiveInfinity,
            NegativeInfinity,
            True,
            False
        }
        
        public static Constant True { get; } = new Constant(1f,  BoolStruct.Instance);
        public static Constant False { get; } = new Constant(0f, BoolStruct.Instance);
        public static Constant BoolNaN { get; } = new Constant(float.NaN, BoolStruct.Instance); // TODO: Find another solution for propagating NaNs and remove this
        public static Constant Zero { get; } = new Constant(0f);
        public static Constant One { get; } = new Constant(1f);
        public static Constant NaN { get; } = new Constant(float.NaN);
        public static Constant PositiveInfinity { get; } = new Constant(float.PositiveInfinity);
        public static Constant NegativeInfinity { get; } = new Constant(float.NegativeInfinity);
        
        public override IEnumerable<Instruction> Dependent { get; } = Array.Empty<Instruction>();
        public override string TypeOf => StructImplementation.Identifier.String;
        public override string SummaryString => NormalizedFormString;
        public override string NormalizedFormString => Value.ToString(CultureInfo.CurrentCulture);
        public override bool Equals(Instruction other) => (other as Constant)?.Value == Value;
        // ReSharper disable once NonReadonlyMemberInGetHashCode
        public override int GetHashCode() => new {Value, Type = StructImplementation}.GetHashCode();
        public string TraceString => NormalizedFormString;
    }
}