namespace Element
{
    using AST;
    
    /// <summary>
    /// Host where the compilation context is persisted between commands.
    /// </summary>
    public class PersistentHost : IHost
    {
        public static Result<PersistentHost> Create(CompilerInput input) => SourceContext.CreateAndLoad(input).Map(context => new PersistentHost(context));

        private PersistentHost(SourceContext context) => _srcContext = context;

        private readonly SourceContext _srcContext;

        public Result Parse(CompilerInput input) => _srcContext.LoadInputFiles(input.Source);

        public Result<float[]> Evaluate(CompilerInput input, string expression) =>
            _srcContext.LoadInputFiles(input.Source)
                    .Bind(() => new Context(_srcContext).EvaluateExpression(expression))
                    .Bind(value => value.Serialize(new Context(_srcContext)))
                    .Bind(serialized => serialized.ToFloatArray(new Context(_srcContext)));

        public Result<string> Typeof(CompilerInput input, string expression) =>
            _srcContext.LoadInputFiles(input.Source)
                    .Bind(() => new Context(_srcContext).EvaluateExpression(expression))
                    .Map(value => value.TypeOf);
    }
}