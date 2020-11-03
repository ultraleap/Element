using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IValue
    {
        string ToString();
        string TypeOf { get; }
        string SummaryString { get; }
        
        Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context);
        IReadOnlyList<ResolvedPort> InputPorts { get; }
        IValue ReturnConstraint { get; }
        Result<IValue> Index(Identifier id, Context context);
        IReadOnlyList<Identifier> Members { get; }
        Result<bool> MatchesConstraint(IValue value, Context context);
        Result<IValue> DefaultValue(Context context);
        void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context);
        Result<IValue> Deserialize(Func<Instruction> nextValue, Context context);

        bool IsFunction { get; }
        bool IsIntrinsic { get; }
        bool IsIntrinsicOfType<TIntrinsicImplementation>() where TIntrinsicImplementation : IIntrinsicImplementation;
        bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic);

        /// <summary>
        /// Get the inner value being wrapped.
        /// If a value is not being wrapped it will return itself.
        /// </summary>
        IValue Inner { get; }
    }

    public abstract class Value : IValue
    {
        private string? _cachedString;

        protected string InputPortsJoined => string.Join(", ", InputPorts);
        
        public sealed override string ToString() => _cachedString ??= SummaryString;
        public virtual string SummaryString => TypeOf;
        public virtual string TypeOf => GetType().Name;
        public virtual Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => context.Trace(EleMessageCode.NotFunction, $"'{this}' cannot be called, it is not a function");
        
        public virtual IReadOnlyList<ResolvedPort> InputPorts => Array.Empty<ResolvedPort>();
        public virtual IValue ReturnConstraint => NothingConstraint.Instance;
        
        public virtual Result<IValue> Index(Identifier id, Context context) => context.Trace(EleMessageCode.NotIndexable, $"'{this}' is not indexable");
        public virtual IReadOnlyList<Identifier> Members => Array.Empty<Identifier>();
        
        public virtual Result<bool> MatchesConstraint(IValue value, Context context) => context.Trace(EleMessageCode.NotConstraint, $"'{this}' cannot be used as a port annotation, it is not a constraint");
        
        public virtual Result<IValue> DefaultValue(Context context) => context.Trace(EleMessageCode.ConstraintNotSatisfied, $"'{this}' cannot produce a default value, only serializable types can produce default values");
        public virtual void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context) => resultBuilder.Append(EleMessageCode.SerializationError, $"'{this}' is not serializable");
        public virtual Result<IValue> Deserialize(Func<Instruction> nextValue, Context context) => context.Trace(EleMessageCode.SerializationError, $"'{this}' cannot be deserialized");
        public virtual bool IsFunction => false;
        public bool IsIntrinsic => IsIntrinsicOfType<IIntrinsicImplementation>();
        public virtual bool IsIntrinsicOfType<TIntrinsicImplementation>() where TIntrinsicImplementation : IIntrinsicImplementation => false;
        public virtual bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic) => false;
        public virtual IValue Inner => this;
    }

    public class ErrorValue : Value
    {
        private ErrorValue(){}
        public static ErrorValue Instance { get; } = new ErrorValue();
    }

    public static class ValueExtensions
    {
        public static Result<IValue> IndexPositionally(this IValue value, int index, Context context)
        {
            var members = value.Members;
            return index < members.Count
                       ? value.Index(members[index], context)
                       : context.Trace(EleMessageCode.ArgumentOutOfRange, $"Cannot access member {index} - '{value}' has {members.Count} members");
        }

        public static Result<IReadOnlyList<IValue>> MemberValues(this IValue value, Context context) => value.Members.Select(m => value.Index(m, context)).ToResultReadOnlyList();

        public static Result<List<Instruction>> Serialize(this IValue value, Context context)
        {
            var result = new ResultBuilder<List<Instruction>>(context, new List<Instruction>());
            value.Serialize(result, context);
            return result.ToResult();
        }

        public static bool IsSerializable(this IValue value, Context context) => value.Serialize(context).IsSuccess;

        public static Result<int> SerializedSize(this IValue value, Context context)
        {
            var size = 0;
            return value.Deserialize(() =>
            {
                size++;
                return Constant.Zero;
            }, context).Map(_ => size); // Discard the value and just check the size
        }

        public static Result<IValue> Deserialize(this IValue value, IEnumerable<Instruction> expressions, Context context) =>
            value.Deserialize(new Queue<Instruction>(expressions).Dequeue, context);

        public static Result<float[]> ToFloats(this IEnumerable<Instruction> expressions, Context context)
        {
            var exprs = expressions as Instruction[] ?? expressions.ToArray();
            var result = new float[exprs.Length];
            for (var i = 0; i < result.Length; i++)
            {
                var expr = exprs[i];
                var val = (expr as Constant)?.Value;
                if (val.HasValue)
                {
                    result[i] = val!.Value;
                }
                else
                {
                    return context.Trace(EleMessageCode.SerializationError, $"Non-constant expression '{expr}' cannot be evaluated to a float");
                }
            }

            return result;
        }

        public static Result<float[]> SerializeToFloats(this IValue value, Context context) =>
            value.Serialize(context)
                 .Bind(serialized => serialized.ToFloats(context));
        
        public static bool IsType<T>(this IValue value, out T result) where T : IValue
        {
            if (value.Inner is T t)
            {
                result = t;
                return true;
            }
            result = default;
            return false;
        }

        public static bool IsInstance(this IValue value, IValue other) => ReferenceEquals(value.Inner, other.Inner);
        
        public static Result<TValueOut> CastInner<TValueIn, TValueOut>(in this Result<TValueIn> input)
            where TValueIn : IValue
            where TValueOut : IValue
            => input.Bind(value => value.IsType<TValueOut>(out var output)
                                       ? new Result<TValueOut>(output)
                                       : throw new InvalidCastException($"'{value}' cannot be casted to {typeof(TValueOut).Name}"));
        
        public static Result<TValueOut> CastInner<TValueOut>(in this Result<IValue> input)
            where TValueOut : IValue
            => input.Bind(value => value.IsType<TValueOut>(out var output)
                                       ? new Result<TValueOut>(output)
                                       : throw new InvalidCastException($"'{value}' cannot be casted to {typeof(TValueOut).Name}"));
    }
}