using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IValue
    {
        string ToString();
        //string NormalFormString { get; }
    }

    public interface ISerializableValue : IValue
    {
        IEnumerable<Element.Expression> Serialize(CompilationContext context);
        ISerializableValue Deserialize(Func<Element.Expression> nextValue, CompilationContext context);
    }

    public static class ValueExtensions
    {
        public static IValue FullyResolveValue(this IValue value, CompilationContext compilationContext)
        {
            var previous = value;
            while (previous is IFunctionSignature fn && fn.IsNullary())
            {
                var result = fn.ResolveCall(Array.Empty<IValue>(), false, compilationContext);
                // ReSharper disable once PossibleUnintendedReferenceComparison
                if (result == previous) break; // Prevent infinite loop if a nullary just returns itself
                previous = result;
            }

            return previous switch
            {
                Element.Expression expr => ConstantFolding.Optimize(expr),
                _ => previous
            };
        }
        
        public static IEnumerable<(Identifier Identifier, IValue Value)> WithoutDiscardedArguments(this IValue[] arguments, Port[] ports)
        {
            var result = new List<(Identifier Identifier, IValue Value)>();
            var variadicArgNumber = 0;
            
            // Keeps iterating until we've checked all arguments that we can
            // There can be more arguments than ports if the function is variadic
            for (var i = 0; i < arguments.Length; i++)
            {
                var arg = arguments[i];
                var port = i < ports.Length ? ports[i] : null;
                // port can be null if we are checking a variadic function
                if (port == Port.VariadicPort || variadicArgNumber > 0)
                {
                    result.Add((new Identifier($"varg{variadicArgNumber}"), arg));
                    variadicArgNumber++;
                }
                else if ((port?.Identifier.HasValue ?? false) && arg != null)
                {
                    result.Add((port.Identifier.Value, arg));
                }
            }

            return result;
        }

        public static IEnumerable<Element.Expression> Serialize(this IValue value, CompilationContext context) =>
            value is ISerializableValue serializableValue
                ? serializableValue.Serialize(context)
                : context.LogError(1, $"'{value}' is not a serializable").Return(Enumerable.Empty<Element.Expression>());

        public static int SerializedSize(this ISerializableValue value, CompilationContext compilationContext)
        {
            var size = 0;
            Element.Expression CountExpr() => size++.Return(Constant.Zero);
            return value?.Deserialize(CountExpr, compilationContext) switch
            {
                CompilationError _ => -1,
                { } => size,
                _ => -1
            };
        }

        public static ISerializableValue Deserialize(this ISerializableValue value, IEnumerable<Element.Expression> expressions,
                                                     CompilationContext context) =>
            value.Deserialize(new Queue<Element.Expression>(expressions).Dequeue, context);

        public static float[]? ToFloatArray(this IEnumerable<Element.Expression> expressions,
                                            CompilationContext compilationContext)
        {
            var success = true;
            var result = expressions.Select(selector: expr =>
                {
                    var val = (expr as Constant)?.Value;
                    if (val.HasValue)
                    {
                        return val.Value;
                    }

                    success = false;
                    compilationContext.LogError(1, $"Non-constant expression '{expr}' cannot be evaluated to a float");
                    return 0;
                }).ToArray();
            return success ? result : null;
        }
    }
}