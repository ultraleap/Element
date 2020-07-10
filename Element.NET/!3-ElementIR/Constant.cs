using System;
using System.Collections.Generic;
using System.Globalization;
using Lexico;

namespace Element.AST
{
    public class Constant : Element.Expression
    {
        // ReSharper disable once UnusedMember.Global - Used by Lexico
        public Constant() {}
        public Constant(float value) => Value = value;
        private Constant(float value, IntrinsicStructImplementation structImplementationOverride) : base(structImplementationOverride) => Value = value;

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
        public static Constant Zero { get; } = new Constant(0f);
        public static Constant One { get; } = new Constant(1f);
        public static Constant NaN { get; } = new Constant(float.NaN);
        public static Constant PositiveInfinity { get; } = new Constant(float.PositiveInfinity);
        public static Constant NegativeInfinity { get; } = new Constant(float.NegativeInfinity);
        
        
        public override IEnumerable<Element.Expression> Dependent { get; } = Array.Empty<Element.Expression>();
        protected override string ToStringInternal() => Value.ToString(CultureInfo.CurrentCulture);
        public override bool Equals(Element.Expression other) => (other as Constant)?.Value == Value;
        // ReSharper disable once NonReadonlyMemberInGetHashCode

        public override int GetHashCode() => new {Value, Type = StructImplementation}.GetHashCode();
    }
}