using System;
using System.Collections.Generic;

namespace Element.AST
{
    public abstract class FunctionDeclaration : Declaration, IFunction
    {
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
        public override string ToString() => $"{Location}:Function";

        public abstract Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
        public abstract IReadOnlyList<Port> Inputs { get; }
        public abstract Port Output { get; }
        IFunction IInstancable<IFunction>.GetDefinition(CompilationContext compilationContext) => this;
    }
}