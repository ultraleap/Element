using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IValue
    {
        IType Type { get; }
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

        public static int GetSerializedSize(this IValue value, CompilationContext compilationContext) => value switch
        {
            { } when value.Type is ISerializableType t => t.Size(value, compilationContext),
            IFunctionSignature fn when fn.IsNullary() => fn.FullyResolveValue(compilationContext).GetSerializedSize(compilationContext),
            _ => compilationContext.LogError(1, $"'{value}' of type '{value.Type}' is not serializable").Return(-1)
        };

        internal static int GetSerializedSize(this IEnumerable<IValue> values, CompilationContext compilationContext)
        {
            var sizes = values.Select(v => v.GetSerializedSize(compilationContext)).ToArray();
            return sizes.Any(i => i <= 0) ? -1 : sizes.Sum();
        }

        public static Element.Expression[] Serialize(this IValue value, CompilationContext compilationContext)
        {
            var size = value.GetSerializedSize(compilationContext);
            if (size <= 0)
            {
                return Array.Empty<Element.Expression>();
            }
            
            var serialized = new Element.Expression[size];
            var position = 0;

            return value.Serialize(ref serialized, ref position, compilationContext)
                       ? serialized
                       : Array.Empty<Element.Expression>();
        }

        internal static bool Serialize(this IValue value, ref Element.Expression[] serialized, ref int position, CompilationContext compilationContext)=>
            value switch
            {
                {} when value.Type is ISerializableType t => t.Serialize(value, ref serialized, ref position, compilationContext),
                IFunctionSignature fn when fn.IsNullary() => fn.FullyResolveValue(compilationContext).Serialize(ref serialized, ref position, compilationContext),
                _ => compilationContext.LogError(1, $"'{value}' of type '{value.Type}' is not serializable").Return(false)
            };
        
        internal static bool Serialize(this IEnumerable<IValue> values, ref Element.Expression[] serialized, ref int position, CompilationContext compilationContext)
        {
            var result = true;
            foreach (var value in values) result &= value.Serialize(ref serialized, ref position, compilationContext);
            return result;
        }

        public static bool TrySerialize(this IValue value, out Element.Expression[] serialized, CompilationContext compilationContext)
            => (serialized = value.Serialize(compilationContext)).Length > 0;

        public static bool TrySerialize(this IValue value, out float[] serialized, CompilationContext compilationContext)
        {
            if (!value.TrySerialize(out Element.Expression[] expressions, compilationContext))
            {
                serialized = null;
                return false;
            }

            serialized = new float[expressions.Length];
            var position = 0;
            var success = true;
            foreach (var expr in expressions)
            {
                if (expr is Constant constant) serialized[position++] = constant.Value;
                else
                {
                    compilationContext.LogError(1, $"Non-constant expression '{expr}' cannot be serialized");
                    position++;
                    success = false;
                }
            }

            return success;
        }

        public static IValue Deserialize(this IValue value, IEnumerable<Element.Expression> expressions, CompilationContext compilationContext) =>
            value.Type is ISerializableType type
                ? type.Deserialize(expressions, compilationContext)
                : compilationContext.LogError(1, $"'{value}' of type '{value.Type}' cannot be deserialized");
        
        public static bool TryDeserialize(this IType type, IEnumerable<Element.Expression> expressions, out IValue value, CompilationContext compilationContext) =>
            (value = type.Deserialize(expressions, compilationContext)) != CompilationError.Instance;
    }
}