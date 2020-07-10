using System.Collections.Generic;

namespace Element.AST
{
    public abstract class IntrinsicImplementation
    {
        public abstract Identifier Identifier { get; }
    }
    
    public abstract class IntrinsicFunctionImplementation : IntrinsicImplementation
    {
        public abstract Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
    }
    
    public abstract class IntrinsicConstraintImplementation : IntrinsicImplementation
    {
        public abstract Result<bool> MatchesConstraint(IValue value, CompilationContext context);
    }
    
    public abstract class IntrinsicStructImplementation : IntrinsicImplementation
    {
        public abstract Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, CompilationContext context);
        public abstract Result<IValue> DefaultValue(CompilationContext _);
        public abstract Result<bool> MatchesConstraint(Struct @struct, IValue value, CompilationContext context);
    }
}