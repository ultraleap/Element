using System;
using System.Collections;
using NUnit.Framework;

namespace Element.NET.TestHelpers
{
    public abstract class HostFixture : FixtureBase
    {
        protected readonly IHost Host = HostArguments.MakeHost();

        protected static readonly bool[] Interpreted = {true, false};

        private Result<float[][]> EvaluateMultiple<T>(CompilerInput compilerInput, T[] expressions, Func<T, Result<float[]>> evaluateT)
        {
            var resultBuilder = new ResultBuilder<float[][]>(new Context(null, compilerInput.Options), new float[expressions.Length][]);

            for (var i = 0; i < expressions.Length; i++)
            {
                var expr = expressions[i];
                var exprResult = evaluateT(expr);
                resultBuilder.Append(exprResult.Then(results =>
                {
                    // ReSharper disable once AccessToModifiedClosure
                    resultBuilder.Result[i] = results;
                }));
            }

            return resultBuilder.ToResult();
        }
        
        protected class FunctionEvaluation
        {
            public FunctionEvaluation(string functionExpression, string callExpression, bool interpreted)
            {
                FunctionExpression = functionExpression;
                CallExpression = callExpression;
                Interpreted = interpreted;
            }

            public string FunctionExpression { get; }
            public string CallExpression { get; }
            public bool Interpreted { get; }
        }

        private void AssertApproxEqual(Result<float[]> controlExpression, Result<float[][]> expectedResults, IComparer comparer) =>
            controlExpression
                .Accumulate(() => expectedResults)
                .Switch((t, messages) =>
                        {
                            var (control, results) = t;
                           LogMessages(messages);
                           foreach (var result in results)
                           {
                               CollectionAssert.AreEqual(control, result, comparer);
                           }
                        },
                       messages => ExpectingSuccess(messages, false));

        protected void AssertApproxEqual(CompilerInput compilerInput, string controlExpression, params float[][] results) =>
            AssertApproxEqual(Host.EvaluateExpression(compilerInput, controlExpression), results, FloatComparer);
        
        protected void AssertApproxEqual(CompilerInput compilerInput, string controlExpression, params string[] expressions)
            => AssertApproxEqual(Host.EvaluateExpression(compilerInput, controlExpression),
                                 EvaluateMultiple(compilerInput, expressions, expr => Host.EvaluateExpression(compilerInput, expr)),
                                 FloatComparer);

        protected void AssertApproxEqualRelaxed(CompilerInput compilerInput, string controlExpression, params string[] expressions) 
            => AssertApproxEqual(Host.EvaluateExpression(compilerInput, controlExpression),
                                 EvaluateMultiple(compilerInput, expressions, expr => Host.EvaluateExpression(compilerInput, expr)),
                                 RelaxedFloatComparer);
        
        protected void AssertApproxEqual(CompilerInput input, FunctionEvaluation controlEval, params FunctionEvaluation[] expected) =>
            AssertApproxEqual(Host.EvaluateFunction(input, controlEval.FunctionExpression, controlEval.CallExpression, controlEval.Interpreted),
                EvaluateMultiple(input, expected, evaluation => Host.EvaluateFunction(input, evaluation.FunctionExpression, evaluation.CallExpression, evaluation.Interpreted)),
                FloatComparer);

        protected void AssertApproxEqual(CompilerInput input, FunctionEvaluation controlEval, params string[] expected) =>
            AssertApproxEqual(Host.EvaluateFunction(input, controlEval.FunctionExpression, controlEval.CallExpression, controlEval.Interpreted),
                              EvaluateMultiple(input, expected, expr => Host.EvaluateExpression(input, expr)),
                              FloatComparer);
        
        protected void AssertTypeof(CompilerInput compilerInput, string expression, string typeStr) =>
            Host.Typeof(compilerInput, expression)
                .Switch((result, messages) =>
                {
                    LogMessages(messages);
                    Assert.That(result, Is.EqualTo(typeStr));
                }, messages => ExpectingSuccess(messages, false));
        
        protected void EvaluateExpectingElementError(CompilerInput compilerInput, EleMessageCode eleMessageCode, string expression)
        {
            var result = Host.EvaluateExpression(compilerInput, expression);
            ExpectingError(result.Messages, result.IsSuccess, MessageExtensions.TypeString, (int)eleMessageCode);
        }
        
        protected void EvaluateExpectingElementError(CompilerInput compilerInput, EleMessageCode eleMessageCode, FunctionEvaluation fn)
        {
            var result = Host.EvaluateFunction(compilerInput, fn.FunctionExpression, fn.CallExpression, fn.Interpreted);
            ExpectingError(result.Messages, result.IsSuccess, MessageExtensions.TypeString, (int)eleMessageCode);
        }
    }
}