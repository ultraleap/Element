using System;
using System.Collections.Generic;

namespace Element.AST
{
    public interface ICallable : IValue
    {
        IValue Call(IValue[] arguments, CompilationContext compilationContext);
    }

    public interface IFunction : ICallable
    {
        Port[]? Inputs { get; }
        Type Output { get; }
    }

    public interface ICompilableFunction : IFunction
    {
        IValue Compile(IScope scope, CompilationContext compilationContext);
        bool IsBeingCompiled { get; set; }
    }

    public static class FunctionExtensions
    {
        public static bool IsNullary(this IFunction function) => function?.Inputs == null || function.Inputs.Length == 0;
        public static IValue ResolveNullaryFunction(this IValue value, CompilationContext compilationContext) =>
            value is IFunction fn && fn.IsNullary()
                ? fn.Call(Array.Empty<IValue>(), compilationContext)
                : value;

        public static IValue ResolveCall(this ICompilableFunction function, IValue[] arguments, Port[] inputs, IScope callScope, CompilationContext compilationContext)
        {
            if (function.IsBeingCompiled) return compilationContext.LogError(11, "Recursion is disallowed");
            function.IsBeingCompiled = true;

            try
            {
                var argsValid = !(arguments?.Length > 0) ||
                                arguments.ValidateArguments(inputs, callScope, compilationContext);
                var result = argsValid
                    ? function.Compile(callScope, compilationContext)
                    : CompilationErr.Instance;
                return result;
            }
            finally
            {
                function.IsBeingCompiled = false;
            }
        }
    }
}