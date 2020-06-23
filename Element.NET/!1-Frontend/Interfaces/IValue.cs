using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    /*public abstract class ElementValue
    {
        public abstract override string ToString();
        //public abstract string NormalFormString { get; }
        
        public virtual Result<ISerializableValue> DefaultValue() => (14, $"'{this}' i");
        public virtual Result<IEnumerable<Element.Expression>> Serialize() => (1, $"'{this}' is not serializable");
        public virtual Result<ISerializableValue> Deserialize(Func<Element.Expression> nextValue) => (1, $"'{this}' cannot be deserialized");
    }*/
    
    
    public interface IValue
    {
        string ToString();
        //string NormalFormString { get; }
    }

    public interface ISerializableValue : IValue
    {
        void Serialize(ResultBuilder<IList<Element.Expression>> resultBuilder);
        Result<ISerializableValue> Deserialize(Func<Element.Expression> nextValue, ITrace trace);
    }

    public static class ValueExtensions
    {
        public static Result<IValue> FullyResolveValue(this IValue value, CompilationContext context) =>
            (value is IFunction fn && fn.IsNullary()
                 ? fn.Call(Array.Empty<IValue>(), context)
                 : new Result<IValue>(value))
            .Map(v => v is Element.Expression expr ? ConstantFolding.Optimize(expr) : v)
            // ReSharper disable once PossibleUnintendedReferenceComparison
            .Bind(v => v != value ? v.FullyResolveValue(context) : new Result<IValue>(v)); // Recurse until the resolved value is the same

        public static IEnumerable<(Identifier Identifier, IValue Value)> WithoutDiscardedArguments(this IEnumerable<IValue> arguments, IEnumerable<Port> ports)
        {
            var argArray = arguments.ToArray();
            var portArray = ports.ToArray();
            
            var result = new List<(Identifier Identifier, IValue Value)>(argArray.Length);
            var variadicArgNumber = 0;

            // Keeps iterating until we've checked all arguments that we can
            // There can be more arguments than ports if the function is variadic
            for (var i = 0; i < argArray.Length; i++)
            {
                var arg = argArray[i];
                var port = i < portArray.Length ? portArray[i] : null;
                // port can be null if we are checking a variadic function
                if (port == Port.VariadicPort || variadicArgNumber > 0)
                {
                    result.Add((new Identifier($"varg{variadicArgNumber}"), arg));
                    variadicArgNumber++;
                }
                else if ((port?.Identifier.HasValue ?? false) && arg != null)
                {
                    result.Add((port!.Identifier!.Value, arg));
                }
            }
            
            return result;
        }

        public static Result<IList<Element.Expression>> Serialize(this IValue value, ITrace trace)
        {
            if (!(value is ISerializableValue serializableValue)) return trace.Trace(MessageCode.SerializationError, $"'{value}' is not a serializable");
            var result = new ResultBuilder<IList<Element.Expression>>(trace, new List<Element.Expression>());
            serializableValue.Serialize(result);
            return result.ToResult();
        }

        public static Result<int> SerializedSize(this ISerializableValue value, ITrace trace)
        {
            var size = 0;
            return value.Deserialize(() =>
            {
                size++;
                return Constant.Zero;
            }, trace).Map(_ => size); // Discard the value and just check the size
        }

        public static Result<ISerializableValue> Deserialize(this ISerializableValue value, IEnumerable<Element.Expression> expressions, ITrace trace) =>
            value.Deserialize(new Queue<Element.Expression>(expressions).Dequeue, trace);

        public static Result<float[]> ToFloatArray(this IEnumerable<Element.Expression> expressions, ITrace trace)
        {
            var exprs = expressions.ToArray();
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
                    return trace.Trace(MessageCode.SerializationError, $"Non-constant expression '{expr}' cannot be evaluated to a float");
                }
            }
            return result;
        }
    }
}