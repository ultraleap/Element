using System;
using System.Collections.Generic;
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

        public Result Parse(CompilerInput input) => _srcContext.LoadCompilerInput(input);

        public Result<float[]> EvaluateExpression(CompilerInput input, string expression, bool interpreted) =>
            Context.CreateFromSourceContext(_srcContext)
                   .ToDefaultBoundaryContext()
                   .Then(context => _srcContext.LoadCompilerInput(input))
                   .Bind(context => context.EvaluateExpression(expression).Map(value => (value, context)))
                   .Bind(t =>
                   {
                       var (value, context) = t;
                       return interpreted
                                  ? value.SerializeToFloats(context)
                                  : value.CompileDynamic(context)
                                         .Bind(compiled => InvokeAndConvertResult(compiled, context));
                   });
        
        private static Result<float[]> InvokeAndConvertResult(Delegate compiledDelegate, BoundaryContext context, params object[] args)
        {
            var result = new List<float>();
            return context.SerializeClrInstance(compiledDelegate.DynamicInvoke(args), result)
                          .Map(() => result.ToArray());
        }

        public Result<float[]> EvaluateFunction(CompilerInput input, string functionExpression, string argumentsAsCallExpression, bool interpreted) =>
            Context.CreateFromSourceContext(_srcContext).ToDefaultBoundaryContext()
                   .Then(context => _srcContext.LoadCompilerInput(input))
                   // Evaluate the function expression and argument call expression
                   .Bind(context => context.EvaluateExpression(functionExpression)
                                           .Accumulate(() => context.Parse<ExpressionChain.CallExpression>(argumentsAsCallExpression, "<evaluation call expression>")
                                                                    .Bind(callExpression => callExpression.Expressions.List.Select(ce => context.EvaluateExpression(ce)).ToResultArray()))
                                           .Map(tuple => (tuple, context)))
                   .Bind(t =>
                   {
                       var ((function, arguments), context) = t;
                       return interpreted
                                  ? function.Call(arguments, context)
                                            .Bind(value => value.SerializeToFloats(context))
                                  : function.CompileDynamic(context)
                                            .Accumulate(() => arguments.Select(arg => arg.CompileDynamic(context).Map(fn => fn.DynamicInvoke())).ToResultArray())
                                            .Bind(compiled => InvokeAndConvertResult(compiled.Item1, context, compiled.Item2));
                   });
        

        public Result<string> Typeof(CompilerInput input, string expression) => Stringify(input, expression, value => value.TypeOf);
        public Result<string> Summary(CompilerInput input, string expression) => Stringify(input, expression, value => value.SummaryString);

        private Result<string> Stringify(CompilerInput input, string expression, Func<IValue, string> stringify) =>
            _srcContext.LoadCompilerInput(input)
                       .Bind(() => Context.CreateFromSourceContext(_srcContext).EvaluateExpression(expression))
                       .Map(stringify);
    }
}