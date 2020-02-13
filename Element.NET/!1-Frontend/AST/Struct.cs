using System;
using Lexico;

namespace Element.AST
{
    public interface IStructBody {}
    
    public class Struct : Item, ICallable
    {
        [Literal("intrinsic"), Optional] private string _intrinsic;
        [Literal("struct"), WhitespaceSurrounded] private Unnamed _;
        [Term] private Declaration _declaration;
        [Term] private IFunctionBody _functionBody;

        public override Identifier Identifier => _declaration.Identifier;
        public override bool Validate(CompilationContext compilationContext)
        {
            throw new NotImplementedException();
        }

        public Port[] Inputs { get; }

        public bool CanBeCached => true;

        public IValue Call(CompilationFrame frame, CompilationContext compilationContext)
        {
            throw new NotImplementedException();
        }
    }
}