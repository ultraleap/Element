using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    /// <summary>
    /// Represents a value in the Element language.
    /// A value could be any of the first-class constructs, e.g. literal numbers, functions, structs, struct instances, constraints.
    /// </summary>
    public interface IValue
    {
        /// <summary>
        /// Get string representation of a Value.
        /// </summary>
        // TODO: Decide on distinction between summary and ToString
        string ToString();
        
        /// <summary>
        /// The type of a value.
        /// This respects struct types in the language as well as types for abstract values such as functions.
        /// e.g.
        /// "5" is "Num"
        /// "Num" is "IntrinsicStruct"
        /// "_(a) = a" is a lambda which has type "ExpressionBodiedFunction"
        /// </summary>
        string TypeOf { get; }
        
        /// <summary>
        /// String which summarises a value.
        /// </summary>
        // TODO: Decide on distinction between summary and ToString
        string SummaryString { get; }
        
        /// <summary>
        /// Call the value with the given arguments.
        /// Will error if the value is not callable (e.g. a function).
        /// </summary>
        Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context);
        
        /// <summary>
        /// Input ports which describe what arguments are valid when calling this value.
        /// Empty array if the value is not callable.
        /// </summary>
        IReadOnlyList<ResolvedPort> InputPorts { get; }
        
        /// <summary>
        /// Return constraint which describes what return values are valid when calling this value.
        /// Nothing constraint if the value is not callable (i.e. calling will always fail at the nothing constraint even if a return value is resolved).
        /// </summary>
        IValue ReturnConstraint { get; }
        
        /// <summary>
        /// Index the value to return a named member value.
        /// Will error if the value is not indexable or the requested member doesn't exist. 
        /// </summary>
        Result<IValue> Index(Identifier id, Context context);
        
        /// <summary>
        /// Members of this value which can be retrieved via indexing.
        /// Empty array if the value is not indexable.
        /// </summary>
        IReadOnlyList<Identifier> Members { get; }
        
        /// <summary>
        /// Determine if another value matches the constraint that this value represents.
        /// Will error if this value does not represent a constraint.
        /// </summary>
        Result<bool> MatchesConstraint(IValue value, Context context);
        
        /// <summary>
        /// Get the default value of this type.
        /// Will error if this value is not a type.
        /// </summary>
        Result<IValue> DefaultValue(Context context);
        
        /// <summary>
        /// Serialize this value to a list of instructions.
        /// Will error if this value is not serializable (cannot be represented at the host boundary).
        /// </summary>
        void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context);
        
        /// <summary>
        /// Deserialize a copy of this value using the instruction providing function.
        /// Will error if this value is not serializable (cannot be represented at the host boundary).
        /// </summary>
        Result<IValue> Deserialize(Func<Instruction> nextValue, Context context);

        /// <summary>
        /// Retrieve the type this value is an instance of.
        /// Will error if the value is not a primitive value or struct instance.
        /// </summary>
        Result<IValue> InstanceType(Context context);
        
        bool IsIntrinsicOfType<TIntrinsicImplementation>() where TIntrinsicImplementation : IIntrinsicImplementation;
        bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic);
        
        
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
        public virtual Result<IValue> InstanceType(Context context) => context.Trace(EleMessageCode.TypeError, $"'{this}' is not an instance of a type");

        public virtual bool IsIntrinsicOfType<TIntrinsicImplementation>() where TIntrinsicImplementation : IIntrinsicImplementation => false;
        public virtual bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic) => false;
    }

    public class ErrorValue : Value
    {
        private ErrorValue(){}
        public static ErrorValue Instance { get; } = new ErrorValue();
    }

    public static class ValueExtensions
    {
        public static IValue Inner(this IValue value) => value is WrapperValue w ? Inner(w.WrappedValue) : value;
        public static bool HasInputs(this IValue value) => value.InputPorts.Count > 0; // TODO: This probably doesn't work for intrinsics with no ports declared?

        public static bool IsCallable(this IValue value, Context context)
            => value.Call(Array.Empty<IValue>(), context)
                    .Match((_, _) => true,
                           messages => messages.All(msg => (EleMessageCode) msg.MessageCode.GetValueOrDefault(0) != EleMessageCode.NotFunction));
        
        public static bool IsType(this IValue value) => value.ReturnConstraint == value;
        public static bool IsIntrinsic(this IValue value) => value.IsIntrinsicOfType<IIntrinsicImplementation>();

        public static Result<IValue[]> GetInputPortDefaults(this IValue function, Context context) => function.InputPorts.Select(c => c.DefaultValue(context)).ToResultArray();
        public static Result<IEnumerable<Instruction>> SerializeAndFlattenValues(this IEnumerable<IValue> values, Context context) =>
            values.Select(v => v.Serialize(context))
                  .MapEnumerable(serializedValues => serializedValues.SelectMany(v => v));

        public static Result<(IValue[] InputDefaultValues, Instruction[] AllDefaultsSerialized)> SerializeAllInputPortDefaults(this IValue function, Context context)
            => function.GetInputPortDefaults(context)
                       .Bind(defaultValues => defaultValues.SerializeAndFlattenValues(context).Map(allSerialized => (defaultValues, allSerialized.ToArray())));
        
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

        public static bool IsInstanceOfType(this IValue value, IValue type, Context context) => value.InstanceType(context)
                                                                                                     .Match( (instanceType, _) => instanceType == type,
                                                                                                       _ => false);

        public static bool AreAllOfInstanceType(this IReadOnlyList<IValue> items, IValue type, Context context) => items.All(value => value.IsInstanceOfType(type, context));

        public static bool InnerIs<T>(this IValue value, out T result) where T : IValue
        {
            if (value.Inner() is T t)
            {
                result = t;
                return true;
            }
            result = default;
            return false;
        }
        
        public static bool HasWrapper<TWrapperValue>(this IValue value, out TWrapperValue? wrapperValue) where TWrapperValue : WrapperValue
        {
            while (value is WrapperValue wrapper)
            {
                if (wrapper is TWrapperValue found)
                {
                    wrapperValue = found;
                    return true;
                }

                value = wrapper.WrappedValue;
            }

            wrapperValue = null;
            return false;
        }
  
        public static bool IsInstance(this IValue value, IValue other) => ReferenceEquals(value.Inner(), other.Inner());
        
        public static Result<TValueOut> CastInner<TValueIn, TValueOut>(in this Result<TValueIn> input)
            where TValueIn : IValue
            where TValueOut : IValue
            => input.Bind(value => value.InnerIs<TValueOut>(out var output)
                                       ? new Result<TValueOut>(output)
                                       : throw new InvalidCastException($"'{value}' cannot be casted to {typeof(TValueOut).Name}"));
        
        public static Result<TValueOut> CastInner<TValueOut>(in this Result<IValue> input)
            where TValueOut : IValue
            => input.Bind(value => value.InnerIs<TValueOut>(out var output)
                                       ? new Result<TValueOut>(output)
                                       : throw new InvalidCastException($"'{value}' cannot be casted to {typeof(TValueOut).Name}"));
    }
}