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
        [Term] private IStructBody _structBody;

        public override Identifier Identifier => _declaration.Identifier;
        public Port[] Inputs => _declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        public bool CanBeCached => true;
        public bool IsIntrinsic => !string.IsNullOrEmpty(_intrinsic);
        public override string ToString() => _declaration.ToString();

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = true;
            if (!IsIntrinsic && Inputs.Length < 1)
            {
                compilationContext.LogError(13, $"Non intrinsic struct '{Identifier}' must have ports");
                success = false;
            }

            if (_declaration.ReturnType != null)
            {
                compilationContext.LogError(17, $"Struct declaration '{Identifier}' cannot have a return type");
                success = false;
            }

            if (_structBody is Scope scope)
            {
                success &= scope.ValidateScope(compilationContext);
            }

            return success;
        }


        public IValue Call(CompilationFrame frame, CompilationContext compilationContext)
        {
            throw new NotImplementedException();
        }
    }
}