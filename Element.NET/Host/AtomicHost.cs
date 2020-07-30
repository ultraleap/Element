namespace Element
{
    /// <summary>
    /// Host where each command executes in a temporary compilation context and no compilation state is persisted between commands.
    /// </summary>
    public class AtomicHost : IHost
    {
        public Result Parse(CompilationInput compilationInput) => (Result)SourceContext.Create(compilationInput);

        public Result<float[]> Evaluate(CompilationInput compilationInput, string expression) =>
            PersistentHost.Create(compilationInput).Bind(host => host.Evaluate(compilationInput, expression));

        public Result<string> Typeof(CompilationInput input, string expression) =>
            PersistentHost.Create(input).Bind(host => host.Typeof(input, expression));
    }
}