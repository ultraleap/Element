using System;
using System.Collections.Generic;
using System.Globalization;
using Lexico;

namespace Element.AST
{
    public class Constant : Element.Expression, IScope
    {
        // ReSharper disable once UnusedMember.Global - Used by Lexico
        public Constant() {}
        public Constant(float value) => Value = value;

        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        [Term] public float Value { get; private set; }
        public static implicit operator float(Constant l) => l.Value;
        
        public static Constant Zero { get; } = new Constant(0f);
        public static Constant One { get; } = new Constant(1f);
        public override IEnumerable<Element.Expression> Dependent { get; } = Array.Empty<Element.Expression>();
        protected override string ToStringInternal() => Value.ToString(CultureInfo.CurrentCulture);
        public override bool Equals(Element.Expression other) => (other as Constant)?.Value == Value;
        // ReSharper disable once NonReadonlyMemberInGetHashCode
        public override int GetHashCode() => Value.GetHashCode();

        public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
            compilationContext.GetIntrinsicsDeclaration<DeclaredStruct>(NumType.Instance)
                   ?.ResolveInstanceFunction(id, this, compilationContext);
    }
}