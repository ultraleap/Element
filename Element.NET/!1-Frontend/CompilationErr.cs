using System;

namespace Element.AST
{
    /// <summary>
    /// A value that results from a failure during compilation. It will be accepted everywhere and generate no further
    /// errors, returning itself from each operation (the error is non-recoverable).
    /// </summary>
    public sealed class CompilationErr : IFunctionSignature, IType, Element.IFunction
    {
        public static CompilationErr Instance { get; } = new CompilationErr();
        private CompilationErr() { }
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;
        public override string ToString() => "<error>";
        string IType.Name => "<error>";
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => false;
        Port[] IFunctionSignature.Inputs { get; } = Array.Empty<Port>();
        Port IFunctionSignature.Output => null;
        IType IValue.Type => Instance;

        
        
        // TODO: Delete these
        PortInfo[] Element.IFunction.Inputs { get; } = null;
        PortInfo[] Element.IFunction.Outputs { get; } = null;
        public Element.IFunction CallInternal(Element.IFunction[] arguments, string output, CompilationContext context) => this;
    }
}