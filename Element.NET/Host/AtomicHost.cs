namespace Element
{
    /// <summary>
    /// Host where each command executes in a temporary compilation context and no compilation state is persisted between commands.
    /// </summary>
    public class AtomicHost : IHost
    {
        public Result Parse(CompilerInput compilerInput) => (Result)SourceContext.CreateAndLoad(compilerInput);

        public Result<float[]> Evaluate(CompilerInput compilerInput, string expression) =>
            PersistentHost.Create(compilerInput.Options).Evaluate(compilerInput, expression);

        public Result<string> Typeof(CompilerInput input, string expression) =>
            PersistentHost.Create(input.Options).Typeof(input, expression);
    }
}