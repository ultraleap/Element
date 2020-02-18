using System;
using System.IO;
using Element.AST;

namespace Element
{
    /// <summary>
    /// Host where each command executes in a temporary compilation context and no compilation state is persisted between commands.
    /// </summary>
    public class AtomicHost : IHost
    {
        public bool ParseFile(in CompilationInput compilationInput, FileInfo file) =>
            CompilationContext.TryCreate(compilationInput, out var context) && context.ParseFile(file);

        public float[] Evaluate(in CompilationInput compilationInput, string expression) =>
            CompilationContext.TryCreate(compilationInput, out var context)
                ? context.Parse(expression, out AST.Expression expressionObject)
                    ?
                    expressionObject.ResolveExpression(context.GlobalScope, context) switch
                    {
                        // TODO: Replace with serialization
                        CompilationErr _ => Array.Empty<float>(),
                        Literal lit => new[] {(float) lit},
                        StructInstance instance => Array.Empty<float>(), // HACK!
                        var result => throw new NotImplementedException($"{result} evaluation not implemented")
                    }
                    : Array.Empty<float>()
                : Array.Empty<float>();
    }
}