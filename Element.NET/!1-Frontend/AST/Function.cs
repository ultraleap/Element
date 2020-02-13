using System;
using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    public interface IFunctionBody {}
    
    public class Function : Item, ICallable
    {
        [Literal("intrinsic"), Optional] private string _intrinsic;
        [Term] private Declaration _declaration;
        [Term] private IFunctionBody _functionBody;

        public override Identifier Identifier => _declaration.Identifier;
        public Port[] Inputs => _declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        public bool CanBeCached => Inputs == null || Inputs.Length == 0;
        public bool IsIntrinsic => !string.IsNullOrEmpty(_intrinsic);
        public override string ToString() => _declaration.ToString();

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = true;

            if (_declaration.ReturnType != null)
            {
                success &= _declaration.ReturnType.Validate(compilationContext);
            }
            
            if (_functionBody is Scope scope)
            {
                success &= scope.ValidateScope(compilationContext, new List<Identifier>{Parser.ReturnIdentifier});
            }

            return success;
        }

        public IValue Call(CompilationFrame frame, CompilationContext compilationContext)
        {
            IValue CompileFunction(Function function, CompilationFrame callSiteFrame) =>
                function._functionBody switch
                {
                    // If a function is a binding (has expression body) we just compile the single expression
                    Binding binding => compilationContext.CompileExpression(binding.Expression, callSiteFrame),

                    // If a function has a scope body we find the return value
                    Scope scope => scope[Parser.ReturnIdentifier] switch
                    {
                        // If the return value is a function, compile it 
                        Function returnFunction => CompileFunction(returnFunction, callSiteFrame.Push(scope)),
                        null => compilationContext.LogError(7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                        // TODO: Add support for returning other items as values - structs and namespaces
                        var nyi => throw new NotImplementedException(nyi.ToString())
                    },
                    _ => CompilationErr.Instance
                };

            return CompileFunction(this, frame);
        }
    }
}