using System;
using System.Linq;
using Element.CLR;

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

        public Result<float[]> EvaluateExpression(CompilerInput input, string expression, bool interpreted)
        {
            var context = new Context(_srcContext);
            return _srcContext.LoadCompilerInput(input)
                              .Bind(_ => context.EvaluateExpression(expression))
                              .Bind(expr => interpreted
                                                ? expr.SerializeToFloats(context)
                                                : expr.CompileDynamic(context)
                                                      .Bind(compiled => InvokeAndConvertResult(compiled, context)));
        }
        
        // TODO: Don't assume result is a single float - replace the result type of the compiled delegate with float array
        private static Result<float[]> InvokeAndConvertResult(Delegate compiledDelegate, Context context, params object[] args) => compiledDelegate.DynamicInvoke(args) switch
        {
            float f => new[]{f},
            bool b => new[]{b ? 1f : 0f},
            {} result => context.Trace(EleMessageCode.InvalidBoundaryFunction, $"'{result}' is not a recognized dynamic return type")
        };

        public Result<float[]> EvaluateFunction(CompilerInput input, string functionExpression, string argumentsAsCallExpression, bool interpreted)
        {
            var context = new Context(_srcContext);
            return _srcContext.LoadCompilerInput(input)
                              // Evaluate the function expression and argument call expression
                              .Bind(_ => context.EvaluateExpression(functionExpression)
                                                .Accumulate(() => context.Parse<ExpressionChain.CallExpression>(argumentsAsCallExpression, "<evaluation call expression>")
                                                                         .Bind(callExpression => callExpression.Expressions.List.Select(ce => context.EvaluateExpression(ce)).ToResultArray())))
                              .Bind(t => interpreted
                                             ? t.Item1.Call(t.Item2, context)
                                                .Bind(value => value.SerializeToFloats(context))
                                             : t.Item1.CompileDynamic(context)
                                                .Accumulate(() => t.Item2.Select(arg => arg.CompileDynamic(context).Map(fn => fn.DynamicInvoke())).ToResultArray())
                                                .Bind(compiled => InvokeAndConvertResult(compiled.Item1, context, compiled.Item2)));
        }

        public Result<string> Typeof(CompilerInput input, string expression) => Stringify(input, expression, value => value.TypeOf);
        public Result<string> Summary(CompilerInput input, string expression) => Stringify(input, expression, value => value.SummaryString);
        public Result<string> NormalForm(CompilerInput input, string expression) => Stringify(input, expression, value => value.NormalizedFormString);

        private Result<string> Stringify(CompilerInput input, string expression, Func<IValue, string> stringify) =>
            _srcContext.LoadCompilerInput(input)
                       .Bind(_ => new Context(_srcContext).EvaluateExpression(expression))
                       .Map(stringify);
    }
}