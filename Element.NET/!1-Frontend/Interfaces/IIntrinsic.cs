using System.Collections.Generic;

namespace Element.AST
{
    public abstract class Intrinsic<TDeclaration> where TDeclaration : Declaration
    {
        public abstract Identifier Identifier { get; }

        public Result<TDeclaration> Declaration(IIntrinsicCache cache) => cache.GetIntrinsic<TDeclaration>(Identifier);
    }
    
    public abstract class IntrinsicFunction : Intrinsic<IntrinsicFunctionDeclaration>
    {
        public abstract Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
        public abstract IReadOnlyList<Port> Inputs { get; }
        public abstract Port Output { get; }
    }
    
    public abstract class IntrinsicConstraint : Intrinsic<IntrinsicConstraintDeclaration>, IConstraint
    {
        public abstract Result<bool> MatchesConstraint(IValue value, CompilationContext compilationContext);
    }
    
    public abstract class IntrinsicType : Intrinsic<IntrinsicStructDeclaration>, IConstraint
    {
        public abstract IReadOnlyList<Port> Fields { get; }
        public abstract Result<IValue> Construct(StructDeclaration declaration, IReadOnlyList<IValue> arguments);
        public abstract Result<ISerializableValue> DefaultValue(CompilationContext context);
        public abstract Result<bool> MatchesConstraint(IValue value, CompilationContext compilationContext);
    }
}