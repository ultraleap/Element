using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    /// <summary>
    /// A value that results from a failure during compilation. It will be accepted everywhere and generate no further
    /// errors, returning itself from each operation (the error is non-recoverable).
    /// </summary>
    public sealed class CompilationError : Element.Expression, IFunctionSignature, IType
    {
        public static CompilationError Instance { get; } = new CompilationError();
        private CompilationError() { }
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;
        protected override string ToStringInternal() => "<error>";
        public ISerializableValue DefaultValue(CompilationContext _) => this;
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => false;
        Port[] IFunctionSignature.Inputs => Array.Empty<Port>();
        Port IFunctionSignature.Output => Port.ReturnPort(this);
        public override IEnumerable<Element.Expression> Dependent => Enumerable.Empty<Element.Expression>();
    }
}