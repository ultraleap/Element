using System;

namespace Element.AST
{
    public abstract class FunctionDeclaration : Declaration, IFunctionSignature
    {
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
        public Port[] Inputs => DeclaredInputs;
        public Port Output => DeclaredOutput;
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;
    }
}