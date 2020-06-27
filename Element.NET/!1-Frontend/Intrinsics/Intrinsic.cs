using System.Collections.Generic;

namespace Element.AST
{
    public abstract class Intrinsic
    {
        public abstract Identifier Identifier { get; }
    }
    
    public abstract class IntrinsicFunctionSignature : Intrinsic
    {
        public override string ToString() => $"{Identifier}:Function";
        public abstract Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
    }
    
    public abstract class IntrinsicConstraint : Intrinsic
    {
        public override string ToString() => $"{Identifier}:Constraint";
        public abstract Result<bool> MatchesConstraint(IValue value, CompilationContext context);
    }
    
    public abstract class IntrinsicType : Intrinsic
    {
        public override string ToString() => $"{Identifier}:Struct";
        public abstract Result<IValue> Construct(StructDeclaration decl, IReadOnlyList<IValue> arguments, CompilationContext context);
        public abstract Result<IValue> DefaultValue(CompilationContext _);
        public abstract Result<bool> MatchesConstraint(StructDeclaration decl, IValue value, CompilationContext context);
    }
}