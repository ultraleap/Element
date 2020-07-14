using System.Collections.Generic;

namespace Element.AST
{
    public abstract class IntrinsicValue : Value, IIntrinsicValue, IIntrinsicImplementation
    {
        protected IntrinsicValue(string? location = null) : base(location){}
        public IIntrinsicImplementation Implementation => this;
        
        // Force intrinsic values to implement non-null identifiers
        Identifier IIntrinsicImplementation.Identifier => _identifier;
        public override Identifier? Identifier => _identifier;
        protected abstract Identifier _identifier { get; }
    }
    
    public interface IIntrinsicValue
    {
        IIntrinsicImplementation Implementation { get; }
    }
    
    public interface IIntrinsicImplementation
    {
        Identifier Identifier { get; }
    }
    
    public interface IIntrinsicFunctionImplementation : IIntrinsicImplementation
    {
        public abstract Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
    }
    
    public interface IIntrinsicConstraintImplementation : IIntrinsicImplementation
    {
        public abstract Result<bool> MatchesConstraint(IValue value, CompilationContext context);
    }
    
    public interface IIntrinsicStructImplementation : IIntrinsicImplementation
    {
        public abstract Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, CompilationContext context);
        public abstract Result<IValue> DefaultValue(CompilationContext _);
        public abstract Result<bool> MatchesConstraint(Struct @struct, IValue value, CompilationContext context);
    }
}