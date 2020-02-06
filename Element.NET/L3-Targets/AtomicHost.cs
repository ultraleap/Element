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

        public float[] Evaluate(in CompilationInput compilationInput, string expression, params float[] arguments) =>
            CompilationContext.TryCreate(compilationInput, out var context)
                ? context.Parse(expression, out AST.Expression expressionObject)
                    ?
                    context.Compile(expressionObject) switch
                    {
                        CompilationErr _ => Array.Empty<float>(),
                        Literal lit => new[] {(float) lit},
                        var result => throw new NotImplementedException($"{result} evaluation not implemented")
                    }
                    : Array.Empty<float>()
                : Array.Empty<float>();
    }
}