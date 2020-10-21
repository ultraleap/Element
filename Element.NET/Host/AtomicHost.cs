namespace Element
{
    /// <summary>
    /// Host where each command executes in a temporary compilation context and no compilation state is persisted between commands.
    /// </summary>
    public class AtomicHost : IHost
    {
        public Result Parse(CompilerInput compilerInput) => (Result)SourceContext.CreateAndLoad(compilerInput);

        public Result<float[]> EvaluateExpression(CompilerInput input, string expression, bool interpreted) =>
            PersistentHost.Create(input.Options).EvaluateExpression(input, expression, interpreted);

        public Result<float[]> EvaluateFunction(CompilerInput input, string functionExpression, string argumentsAsCallExpression, bool interpreted) =>
            PersistentHost.Create(input.Options).EvaluateFunction(input, functionExpression, argumentsAsCallExpression, interpreted);

        public Result<string> Typeof(CompilerInput input, string expression) =>
            PersistentHost.Create(input.Options).Typeof(input, expression);

        public Result<string> Summary(CompilerInput input, string expression) =>
            PersistentHost.Create(input.Options).Summary(input, expression);
    }
}