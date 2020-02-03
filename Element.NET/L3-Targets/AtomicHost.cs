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
                ? context.GlobalScope.TryGetValue(functionName, context, out var result)
                    ? new[] {((Literal)result).Value} // TODO: Don't cast to literal here!
                    : Array.Empty<float>()
                : Array.Empty<float>();
    }
}