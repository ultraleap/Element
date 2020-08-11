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
        string NormalizedFormString { get; }
        
        Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context);
        IReadOnlyList<ResolvedPort> InputPorts { get; }
        IValue ReturnConstraint { get; }
        Result<IValue> Index(Identifier id, Context context);
        IReadOnlyList<Identifier> Members { get; }
        Result<bool> MatchesConstraint(IValue value, Context context);
        Result<IValue> DefaultValue(Context context);
        void Serialize(ResultBuilder<List<Element.Instruction>> resultBuilder, Context context);
        Result<IValue> Deserialize(Func<Element.Instruction> nextValue, Context context);
    }

    public abstract class Value : IValue
    {
        private string? _cachedString;
        public sealed override string ToString() => _cachedString ??= SummaryString;
        public virtual string SummaryString => TypeOf;
        public virtual string TypeOf => GetType().Name;
        public virtual string NormalizedFormString => SummaryString;
        public virtual Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => context.Trace(MessageCode.NotFunction, $"'{this}' cannot be called, it is not a function");
        
        public virtual IReadOnlyList<ResolvedPort> InputPorts => Array.Empty<ResolvedPort>();
        public virtual IValue ReturnConstraint => NothingConstraint.Instance;
        
        public virtual Result<IValue> Index(Identifier id, Context context) => context.Trace(MessageCode.NotIndexable, $"'{this}' is not indexable");
        public virtual IReadOnlyList<Identifier> Members => Array.Empty<Identifier>();
        
        public virtual Result<bool> MatchesConstraint(IValue value, Context context) => context.Trace(MessageCode.NotConstraint, $"'{this}' cannot be used as a port annotation, it is not a constraint");
        
        public virtual Result<IValue> DefaultValue(Context context) => context.Trace(MessageCode.ConstraintNotSatisfied, $"'{this}' cannot produce a default value, only serializable types can produce default values");
        public virtual void Serialize(ResultBuilder<List<Element.Instruction>> resultBuilder, Context context) => resultBuilder.Append(MessageCode.SerializationError, $"'{this}' is not serializable");
        public virtual Result<IValue> Deserialize(Func<Element.Instruction> nextValue, Context context) => context.Trace(MessageCode.SerializationError, $"'{this}' cannot be deserialized");
    }

    public static class ValueExtensions
    {
        public static bool IsFunction(this IValue value) => value.InputPorts.Count > 0;
        
        public static bool IsIntrinsic<TIntrinsicImplementation>(this IValue value)
            where TIntrinsicImplementation : IIntrinsicImplementation =>
            value is IIntrinsicValue c
            && c.Implementation.GetType() == typeof(TIntrinsicImplementation);
        
        public static bool IsIntrinsic(this IValue value, IIntrinsicImplementation implementation) =>
            value is IIntrinsicValue intrinsic
            && intrinsic.Implementation == implementation;

        public static Result<IValue> IndexPositionally(this IValue value, int index, Context context)
        {
            var members = value.Members;
            return index < members.Count
                       ? value.Index(members[index], context)
                       : context.Trace(MessageCode.ArgumentOutOfRange, $"Cannot access member {index} - '{value}' has {members.Count} members");
        }

        public static Result<List<Element.Instruction>> Serialize(this IValue value, Context context)
        {
            var result = new ResultBuilder<List<Element.Instruction>>(context, new List<Element.Instruction>());
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

        public static Result<IValue> Deserialize(this IValue value, IEnumerable<Element.Instruction> expressions, Context context) =>
            value.Deserialize(new Queue<Element.Instruction>(expressions).Dequeue, context);

        public static Result<float[]> ToFloatArray(this IEnumerable<Element.Instruction> expressions, Context context)
        {
            var exprs = expressions as Element.Instruction[] ?? expressions.ToArray();
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
                    return context.Trace(MessageCode.SerializationError, $"Non-constant expression '{expr}' cannot be evaluated to a float");
                }
            }

            return result;
        }
    }
}