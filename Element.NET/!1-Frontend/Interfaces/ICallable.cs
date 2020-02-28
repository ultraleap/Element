using System;

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

    public static class FunctionExtensions
    {
        public static bool IsNullary(this IFunction function) => function?.Inputs == null || function.Inputs.Length == 0;

        public static IValue ResolveNullaryFunction(this IValue value, CompilationContext compilationContext) =>
            value is IFunction function && function.IsNullary()
                ? function.Call(Array.Empty<IValue>(), compilationContext)
                : value;
    }
}