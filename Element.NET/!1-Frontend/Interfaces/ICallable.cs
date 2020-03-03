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

    public interface ICompilableFunction : IFunction
    {
        IValue Compile(IScope scope, CompilationContext compilationContext);
    }

    public static class FunctionExtensions
    {
        public static bool IsNullary(this IFunction function) => function?.Inputs == null || function.Inputs.Length == 0;

        public static IValue ResolveNullaryFunction(this IValue value, CompilationContext compilationContext) =>
            value is IFunction function && function.IsNullary()
                ? function.Call(Array.Empty<IValue>(), compilationContext)
                : value;

        public static IValue CompileFunction(this IScope parentScope, object body, CompilationContext compilationContext)  =>
            body switch
            {
                // If a function is a binding (has expression body) we just compile the single expression
                Binding binding => binding.Expression.ResolveExpression(parentScope, compilationContext),
                // If a function has a scope body we find the return value
                Scope scope => scope[Parser.ReturnIdentifier, compilationContext] switch
                {
                    // If the return value is a function, compile it
                    ICompilableFunction returnFunction => returnFunction.Compile(parentScope, compilationContext),
                    null => compilationContext.LogError(7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                    var nyi => throw new NotImplementedException(nyi.ToString())
                },
                _ => CompilationErr.Instance
            };
    }
}