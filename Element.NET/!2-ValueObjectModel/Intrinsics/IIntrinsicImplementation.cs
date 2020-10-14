using System.Collections.Generic;

namespace Element.AST
{
    public abstract class IntrinsicValue : Value, IIntrinsicValue, IIntrinsicImplementation
    {
        public IIntrinsicImplementation Implementation => this;
        public abstract Identifier Identifier { get; }
        Identifier IIntrinsicImplementation.Identifier => Identifier;
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
        public abstract Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context);
        public bool IsVariadic { get; }
    }
    
    public interface IIntrinsicConstraintImplementation : IIntrinsicImplementation
    {
        public abstract Result<bool> MatchesConstraint(IValue value, Context context);
    }
    
    public interface IIntrinsicStructImplementation : IIntrinsicImplementation
    {
        public abstract Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, Context context);
        public abstract Result<IValue> DefaultValue(Context _);
        public abstract Result<bool> MatchesConstraint(Struct @struct, IValue value, Context context);
    }
}