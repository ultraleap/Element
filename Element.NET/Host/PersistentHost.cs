using Element.AST;

namespace Element
{
    /// <summary>
    /// Host where the compilation context is persisted between commands.
    /// </summary>
    public class PersistentHost : IHost
    {
        public static Result<PersistentHost> Create(CompilationInput input) => SourceContext.Create(input).Map(context => new PersistentHost(context));

        private PersistentHost(SourceContext context) => _srcContext = context;

        private readonly SourceContext _srcContext;

        public Result Parse(CompilationInput input) => _srcContext.ApplyExtraInput(input);

        public Result<float[]> Evaluate(CompilationInput input, string expression) =>
            _srcContext.ApplyExtraInput(input)
                    .Bind(() => new Context(_srcContext).EvaluateExpression(expression))
                    .Bind(value => value.Serialize(new Context(_srcContext)))
                    .Bind(serialized => serialized.ToFloatArray(new Context(_srcContext)));

        public Result<string> Typeof(CompilationInput input, string expression) =>
            _srcContext.ApplyExtraInput(input)
                    .Bind(() => new Context(_srcContext).EvaluateExpression(expression))
                    .Map(value => value.TypeOf);
    }
}