using System;

namespace Element
{
    using AST;
    
    /// <summary>
    /// Host where the compilation context is persisted between commands.
    /// </summary>
    public class PersistentHost : IHost
    {
        public static PersistentHost Create(CompilerOptions options) => new PersistentHost(new SourceContext(options));

        private PersistentHost(SourceContext context) => _srcContext = context;

        private readonly SourceContext _srcContext;

        public Result Parse(CompilerInput input) => (Result)_srcContext.LoadCompilerInput(input);

        public Result<float[]> Evaluate(CompilerInput input, string expression) =>
            _srcContext.LoadCompilerInput(input)
                    .Bind(_ => new Context(_srcContext).EvaluateExpression(expression))
                    .Bind(value => value.SerializeToFloats(new Context(_srcContext)));

        public Result<string> Typeof(CompilerInput input, string expression) => Stringify(input, expression, value => value.TypeOf);
        public Result<string> Summary(CompilerInput input, string expression) => Stringify(input, expression, value => value.SummaryString);
        public Result<string> NormalForm(CompilerInput input, string expression) => Stringify(input, expression, value => value.NormalizedFormString);

        private Result<string> Stringify(CompilerInput input, string expression, Func<IValue, string> stringify) =>
            _srcContext.LoadCompilerInput(input)
                       .Bind(_ => new Context(_srcContext).EvaluateExpression(expression))
                       .Map(stringify);
    }
}