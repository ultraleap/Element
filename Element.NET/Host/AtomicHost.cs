using System;

namespace Element
{
    /// <summary>
    /// Host where each command executes in a temporary compilation context and no compilation state is persisted between commands.
    /// </summary>
    public class AtomicHost : IHost
    {
        public bool Parse(CompilationInput compilationInput) => SourceContext.TryCreate(compilationInput, out _);
        public (bool Success, float[] Result) Evaluate(CompilationInput compilationInput, string expression) => PersistentHost.TryCreate(compilationInput, out var host)
                                                                                                                    ? host.Evaluate(compilationInput, expression)
                                                                                                                    : (false, Array.Empty<float>());
        public (bool Success, string Result) Typeof(CompilationInput input, string expression) => PersistentHost.TryCreate(input, out var host)
                                                                                                      ? host.Typeof(input, expression) 
                                                                                                      : (false, string.Empty);
    }
}