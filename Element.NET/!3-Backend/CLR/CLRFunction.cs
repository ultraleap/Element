using System;
using System.Linq;
using System.Linq.Expressions;
using ElementExpression = Element.Expression;
using LinqExpression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public static partial class CLRFunction
    {
        public static Delegate? Compile(this AST.IFunction function, Type delegateType, SourceContext sourceContext)
        {
            if (function == null) throw new ArgumentNullException(nameof(function));
            if (delegateType == null) throw new ArgumentNullException(nameof(delegateType));
            if (sourceContext == null) throw new ArgumentNullException(nameof(sourceContext));

            var context = sourceContext.MakeCompilationContext();

            // Check inputs/outputs are boundary-compatible
            var isNullary = function.Inputs.Length < 1;

            // Check return type/single out parameter of delegate
            var method = delegateType.GetMethod(nameof(Action.Invoke));
            var delegateParameters = method.GetParameters();
            var delegateReturn = method.ReturnParameter;
            if (delegateParameters.Any(p => p.IsOut) || method.ReturnType == typeof(void))
            {
                context.LogError(10, $"{delegateType} cannot have out parameters and must have non-void return type");
                return null;
            }

            // Create parameter expressions
            var parameterExpressions = delegateParameters.Select(p => LinqExpression.Parameter(p.ParameterType, p.Name)).ToArray();

            // Match function inputs to parameter expressions
            var arguments = function.Inputs.Select(f =>
                                    {
                                        ParameterExpression p;
                                        return (p = Array.Find(parameterExpressions, i => i.Name == f.Identifier)) == null
                                                   ? context.LogError(10, $"Unable to bind {function}'s input {f} - could not find matching parameter name on delegate")
                                                   : boundaryMap.ToInput(p, boundaryMap, context);
                                    })
                                    .ToArray();

            // Reduce function to expressions (invoke mid-compiler)
            var output = function.Call(arguments, context);

            // TODO: Compile to linq expressions

            // Compile delegate

            return null;
        }
    }
}