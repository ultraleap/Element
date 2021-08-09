using System;
using System.Collections.Generic;
using ResultNET;

namespace Element.AST
{
    public abstract class Value : IValue
    {
        private string? _cachedString;

        protected string InputPortsJoined => string.Join(", ", InputPorts);
        
        public sealed override string ToString() => _cachedString ??= SummaryString;
        public virtual string SummaryString => TypeOf;
        public virtual string TypeOf => GetType().Name;
        public virtual Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => context.Trace(ElementMessage.NotFunction, $"'{this}' cannot be called, it is not a function");
        public virtual IReadOnlyList<ResolvedPort> InputPorts => Array.Empty<ResolvedPort>();
        public virtual IValue ReturnConstraint => NothingConstraint.Instance;
        public virtual Result<IValue> Index(Identifier id, Context context) => context.Trace(ElementMessage.NotIndexable, $"'{this}' is not indexable");
        public virtual IReadOnlyList<Identifier> Members => Array.Empty<Identifier>();
        public virtual Result MatchesConstraint(IValue value, Context context) => context.Trace(ElementMessage.NotConstraint, $"'{this}' is not a constraint");
        public virtual Result<IValue> DefaultValue(Context context) => context.Trace(ElementMessage.ConstraintNotSatisfied, $"'{this}' cannot produce a default value, only serializable types can produce default values");
        public virtual void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context) => resultBuilder.Append(ElementMessage.SerializationError, $"'{this}' is not serializable");
        public virtual Result<IValue> Deserialize(Func<IIntrinsicStructImplementation, Instruction> nextValue, Context context) => context.Trace(ElementMessage.SerializationError, $"'{this}' cannot be deserialized");
        public virtual Result<IValue> InstanceType(Context context) => context.Trace(ElementMessage.TypeError, $"'{this}' is not an instance of a type");

        public virtual bool IsIntrinsicOfType<TIntrinsicImplementation>() where TIntrinsicImplementation : IIntrinsicImplementation => false;
        public virtual bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic) => false;
    }
}