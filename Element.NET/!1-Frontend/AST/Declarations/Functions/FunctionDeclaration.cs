using System;
using System.Collections.Generic;

namespace Element.AST
{
    public abstract class FunctionSignatureDeclaration : Declaration
    {
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Block), typeof(Terminal)};
        public override string ToString() => $"{Location}:Function";

        public IReadOnlyList<Port> Inputs => DeclaredInputs;
        public Port Output => DeclaredOutput;
    }
}