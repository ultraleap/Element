using System.Collections.Generic;

namespace Element.AST
{
    public class Closure : Value
    {
        public override string ToString()
        {
            throw new System.NotImplementedException();
        }

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context)
        {
            
        }
    }

    public class Struct : Value, IScope, IFunctionSignature
    {
        public override string ToString()
        {
            throw new System.NotImplementedException();
        }

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context)
        {
            
        }

        public IReadOnlyList<Port> Inputs { get; }
        public Port Output { get; }

        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context)
        {
            
        }

        public override Result<IValue> Index(Identifier id, CompilationContext context)
        {
            
        }

        public Result<IValue> Lookup(Identifier id, CompilationContext context)
        {
            throw new System.NotImplementedException();
        }

        public override Result<IValue> DefaultValue(CompilationContext context)
        {
            
        }

        public override IReadOnlyList<IValue> Members { get; }
    }
    
    public class Constraint : Value
    {
        public override string ToString()
        {
            throw new System.NotImplementedException();
        }

        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context)
        {
            
        }
    }
}