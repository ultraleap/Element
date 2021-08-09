using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ResultNET;

namespace Element.AST
{
    public static class ValueExtensions
    {
        public static IValue Inner(this IValue value) => value is WrapperValue w ? Inner(w.WrappedValue) : value;
        public static bool HasInputs(this IValue value) => value.InputPorts.Count > 0; // TODO: This probably doesn't work for intrinsics with no ports declared?

        public static bool IsCallable(this IValue value, Context context)
            => value.Call(Array.Empty<IValue>(), context)
                    .Match((_, __) => true,
                         messages => messages.All(msg => msg.Info != ElementMessage.NotFunction));
        
        public static bool IsType(this IValue value) => value.ReturnConstraint == value;
        public static bool IsIntrinsic(this IValue value) => value.IsIntrinsicOfType<IIntrinsicImplementation>();

        public static Result<IValue[]> GetInputPortDefaults(this IValue function, Context context) => function.InputPorts.Select(c => c.DefaultValue(context)).ToResultArray();
        public static Result<IEnumerable<Instruction>> SerializeAndFlattenValues(this IEnumerable<IValue> values, Context context) =>
            values.Select(v => v.Serialize(context))
                  .MapEnumerable(serializedValues => serializedValues.SelectMany(v => v));

        public static Result<(IValue[] InputDefaultValues, Instruction[] AllDefaultsSerialized)> SerializeAllInputPortDefaults(this IValue function, Context context)
            => function.GetInputPortDefaults(context)
                       .Bind(defaultValues => defaultValues.SerializeAndFlattenValues(context)
                                                           .Map(allSerialized => (defaultValues, allSerialized.ToArray())));

        public static Result<IValue> IndexPositionally(this IValue value, int index, Context context)
        {
            var members = value.Members;
            return index < members.Count
                ? value.Index(members[index], context)
                : context.Trace(ElementMessage.ArgumentOutOfRange, $"Cannot access member {index} - '{value}' has {members.Count} members");
        }

        public static Result<IReadOnlyList<(Identifier Identifier, IValue Value)>> MemberValues(this IValue value, Context context) => value.Members.Select(m => value.Index(m, context).Map(v => (m, v))).ToResultReadOnlyList();

        public static Result<List<Instruction>> Serialize(this IValue value, Context context)
        {
            var result = new ResultBuilder<List<Instruction>>(context, new List<Instruction>());
            value.Serialize(result, context);
            return result.ToResult();
        }

        public static bool IsSerializable(this IValue value, Context context) => value.Serialize(context).IsSuccess;

        public static Result<IValue> Deserialize(this IValue value, Func<Instruction> nextInstruction, Context context) =>
            value.Deserialize(_ => nextInstruction(), context);

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
                    return context.Trace(ElementMessage.SerializationError, $"Non-constant expression '{expr}' cannot be evaluated to a float");
                }
            }

            return result;
        }

        public static Result<float[]> SerializeToFloats(this IValue value, Context context) =>
            value.Serialize(context)
                 .Bind(serialized => serialized.ToFloats(context));

        public static bool IsInstanceOfType(this IValue value, IValue type, Context context) => value.InstanceType(context)
                                                                                                     .Match( (instanceType, _) => instanceType.Inner() == type.Inner(),
                                                                                                          _ => false);

        public static Result<T> VerifyValuesAreAllOfInstanceType<T>(this IReadOnlyList<IValue> items, IValue type, Func<Result<T>> continuation, Context context)
        {
            var errorString = new StringBuilder($"Expected all items to be of type '{type}' - the following items are different types:");
            var isError = false;
            for (var index = 0; index < items.Count; index++)
            {
                var value = items[index];
                if (!value.IsInstanceOfType(type, context))
                {
                    errorString.AppendLine($"[{index}]: '{value}'");
                    isError = true;
                }
            }

            return !isError
                ? continuation()
                : context.Trace(ElementMessage.ExpectedHomogenousItems, errorString.ToString());
        }

        public static bool InnerIs<T>(this IValue value, out T result) where T : IValue
        {
            if (value.Inner() is T t)
            {
                result = t;
                return true;
            }
            result = default!;
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
        
        /// <summary>
        /// Enumerates top-level and nested declarations that match the filter, resolving them to a list of ValueWithLocation.
        /// Will not recurse into function scopes.
        /// </summary>
        public static Result<List<ValueWithLocation>> EnumerateValues(this IDeclarationScope declarationScope, Context context, Predicate<Declaration>? declarationFilter = null, Predicate<ValueWithLocation>? resolvedValueFilter = null)
        {
            var builder = new ResultBuilder<List<ValueWithLocation>>(context, new List<ValueWithLocation>());
            var idStack = new Stack<Identifier>();

            void Recurse(IScope? parentScope, IDeclarationScope declScope)
            {
                builder.Append(declScope.ResolveBlock(parentScope, context)
                                        .And(ResolveBlockValues));
                
                void ResolveBlockValues(IScope containingScope)
                {
                    foreach (var decl in declScope.Declarations)
                    {
                        idStack.Push(decl.Identifier);
                        if (declarationFilter?.Invoke(decl) ?? true)
                        {
                            void AddResolvedValueToResults(IValue v)
                            {
                                var resolvedValue = new ValueWithLocation(idStack.Reverse().ToArray(), v, decl.IndexInSource, decl.SourceInfo);
                                if (resolvedValueFilter?.Invoke(resolvedValue) ?? true) builder.Result.Add(resolvedValue);
                            }

                            builder.Append(decl.Resolve(containingScope, context).And(AddResolvedValueToResults));
                        }

                        if (decl.Body is IDeclarationScope childScope)
                        {
                            void RecurseIntoChildScope(IScope resolvedBlockScope) => Recurse(resolvedBlockScope, childScope);

                            builder.Append(childScope.ResolveBlock(containingScope, context).And(RecurseIntoChildScope));
                        }

                        idStack.Pop();
                    }
                }
            }

            builder.Append(declarationScope.ResolveBlock(null, context)
                                           .And(scope => Recurse(scope, declarationScope)));
            return builder.ToResult();
        }
    }
}