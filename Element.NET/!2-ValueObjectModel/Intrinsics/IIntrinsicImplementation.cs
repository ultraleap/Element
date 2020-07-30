using System.Collections.Generic;

namespace Element.AST
{
    public abstract class IntrinsicValue : Value, IIntrinsicValue, IIntrinsicImplementation
    {
        public IIntrinsicImplementation Implementation => this;
        public abstract Identifier Identifier { get; }
        public override string SummaryString => $"{Identifier}:{TypeOf}";
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
        public bool IsVariadic { get; }
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