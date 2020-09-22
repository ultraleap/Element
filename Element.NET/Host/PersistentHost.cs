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

        public Result<float[]> EvaluateExpression(CompilerInput input, string expression) =>
            _srcContext.LoadCompilerInput(input)
                       .Bind(_ => new Context(_srcContext).EvaluateExpression(expression))
                       .Bind(value => value.SerializeToFloats(new Context(_srcContext)));

        public Result<float[]> EvaluateFunction(CompilerInput input, string functionExpression, string argumentsAsCallExpression, bool interpreted)
        { 
            var context = new Context(_srcContext);
            return _srcContext.LoadCompilerInput(input)
                              // Evaluate the function expression and argument call expression
                              .Bind(_ => context.EvaluateExpression(functionExpression)
                                                .Accumulate(() => context.Parse<ExpressionChain.CallExpression>(argumentsAsCallExpression, "<input call arguments>")
                                                                         .Bind(callExpression => callExpression.Expressions.List.Select(ce => context.EvaluateExpression(ce)).ToResultArray())))
                              .Bind(t =>
                              {
                                  var (function, arguments) = t;
                                  return interpreted
                                             ? function.Call(arguments, context)
                                                       .Bind(value => value.SerializeToFloats(context))
                                             : function.CompileDynamic(context)
                                                       .Accumulate(() => arguments.Select(arg => arg.CompileDynamic(context).Map(fn => fn.DynamicInvoke())).ToResultArray())
                                                       // TODO: Don't assume result is a single float - replace the result type of the compiled delegate with float array
                                                       .Bind(compiled => new Result<float[]>(new []{(float)compiled.Item1.DynamicInvoke(compiled.Item2)}));
                              });
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