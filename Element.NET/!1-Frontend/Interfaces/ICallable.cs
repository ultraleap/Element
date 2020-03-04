using System;
using System.Linq;

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
        ICompilableFunction Clone(CompilationContext compilationContext);
    }

    public static class FunctionExtensions
    {
        public static bool IsNullary(this IFunction function) => function?.Inputs == null || function.Inputs.Length == 0;

        public static IValue ResolveNullaryFunction(this IValue value, CompilationContext compilationContext) =>
            value is IFunction function && function.IsNullary()
                ? function.Call(Array.Empty<IValue>(), compilationContext)
                : value;

        public static IValue ResolveCall(this ICompilableFunction function, IValue[] arguments,
            ref IScope captureScope, ref bool hasRecursed, Port[] inputs, IScope? childScope, IScope parentScope,
            CompilationContext compilationContext)
        {
            if (hasRecursed) return compilationContext.LogError(11, "Recursion is disallowed");

            if (arguments?.Length > 0)
            {
                // If there are any arguments we need to interject a capture scope to store them
                // The capture scope will be indexed before the parent scope when indexing a declared scope
                // Thus the order of indexing for an item becomes "Child -> Captures -> Parent"
                captureScope ??= new ArgumentCaptureScope(parentScope, arguments.Select((arg, index) => (inputs[index].Identifier, arg)));
            }

            hasRecursed = true;

            var callScope = childScope ?? captureScope ?? parentScope;
            var result = arguments.ValidateArguments(inputs, callScope, compilationContext)
                ? function.Compile(callScope, compilationContext)
                : CompilationErr.Instance;

            captureScope = null;
            hasRecursed = false;

            return result;
        }
    }
}