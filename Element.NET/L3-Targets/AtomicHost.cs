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
        public bool ParseFile(in CompilationInput compilationInput, in FileInfo file) =>
            CompilationContext.TryCreate(compilationInput, out var context) && context.ParseFile(file);

        public float[] Execute(in CompilationInput compilationInput, in string functionName, params float[] functionArgs) =>
            CompilationContext.TryCreate(compilationInput, out var context)
                ? context.GlobalScope.Compile(functionName, context) switch
                {
                    CompilationErr _ => Array.Empty<float>(),
                    Literal lit => new[] {(float)lit},
                    var result => throw new NotImplementedException()
                }
                : Array.Empty<float>();
    }
}