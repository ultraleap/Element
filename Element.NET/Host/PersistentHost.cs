using Element.AST;

namespace Element
{
    /// <summary>
    /// Host where the compilation context is persisted between commands.
    /// </summary>
    public class PersistentHost : IHost
    {
        public static Result<PersistentHost> Create(CompilationInput input) => SourceContext.Create(input).Map(context => new PersistentHost(context));

        private PersistentHost(SourceContext context) => _context = context;

        private readonly SourceContext _context;

        public Result Parse(CompilationInput input) => _context.ApplyExtraInput(input);

        public Result<float[]> Evaluate(CompilationInput input, string expression) =>
            _context.ApplyExtraInput(input)
                    .Bind(() => _context.EvaluateExpression(expression))
                    .Bind(value => value.Serialize(new CompilationContext(_context)))
                    .Bind(serialized => serialized.ToFloatArray(_context));

        public Result<string> Typeof(CompilationInput input, string expression) =>
            _context.ApplyExtraInput(input)
                    .Bind(() => _context.EvaluateExpression(expression))
                    .Map(value => value.ToString());
    }
}