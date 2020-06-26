using System.Collections.Generic;

namespace Element.AST
{
    public abstract class Intrinsic
    {
        public abstract Identifier Identifier { get; }
        protected Result<TDeclaration> Declaration<TDeclaration>(IIntrinsicCache cache)
            where TDeclaration : Declaration =>
            cache.GetIntrinsic<TDeclaration>(Identifier);
    }
    
    public abstract class IntrinsicFunction : Intrinsic
    {
        public abstract IReadOnlyList<Port> Inputs { get; }
        public abstract Port Output { get; }
        public abstract Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
        public Result<IntrinsicFunctionDeclaration> Declaration(IIntrinsicCache cache) => Declaration<IntrinsicFunctionDeclaration>(cache);
    }
    
    public abstract class IntrinsicConstraint : Intrinsic, IConstraint
    {
        public abstract Result<bool> MatchesConstraint(IValue value, CompilationContext context);
        public Result<IntrinsicConstraintDeclaration> Declaration(IIntrinsicCache cache) => Declaration<IntrinsicConstraintDeclaration>(cache);
    }
    
    public abstract class IntrinsicType : Intrinsic, IConstraint
    {
        public abstract IReadOnlyList<Port> Fields { get; }
        public abstract Result<IValue> Construct(IReadOnlyList<IValue> arguments, CompilationContext context);
        public abstract Result<ISerializableValue> DefaultValue(CompilationContext context);
        public abstract Result<bool> MatchesConstraint(IValue value, CompilationContext context);
        public Result<IntrinsicStructDeclaration> Declaration(IIntrinsicCache cache) => Declaration<IntrinsicStructDeclaration>(cache);
    }
}