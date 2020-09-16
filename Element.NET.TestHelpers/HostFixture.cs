using System.Collections;
using NUnit.Framework;

namespace Element.NET.TestHelpers
{
    public abstract class HostFixture : FixtureBase
    {
        protected readonly IHost Host = HostArguments.MakeHost();
        
        private Result<float[]> EvaluateControlExpression(CompilerInput compilerInput, string controlExpression) 
            => Host.Evaluate(compilerInput, controlExpression);

        private Result<float[][]> EvaluateExpressions(CompilerInput compilerInput, string[] expressions)
        {
            var resultBuilder = new ResultBuilder<float[][]>(new Context(null, compilerInput.Options), new float[expressions.Length][]);

            for (var i = 0; i < expressions.Length; i++)
            {
                var expr = expressions[i];
                var exprResult = Host.Evaluate(compilerInput, expr);
                resultBuilder.Append(exprResult.Then(results =>
                {
                    // ReSharper disable once AccessToModifiedClosure
                    resultBuilder.Result[i] = results;
                }));
            }

            return resultBuilder.ToResult();
        }

        private void AssertApproxEqual(CompilerInput compilerInput, string controlExpression, IComparer comparer, params float[][] results) =>
            EvaluateControlExpression(compilerInput, controlExpression)
                .Switch((control, messages) =>
                       {
                           LogMessages(messages);
                           foreach (var result in results)
                           {
                               CollectionAssert.AreEqual(control, result, comparer);
                           }
                       },
                       messages => ExpectingSuccess(messages, false));

        private void AssertApproxEqual(CompilerInput compilerInput, string controlExpression, IComparer comparer, params string[] expressions) =>
            EvaluateControlExpression(compilerInput, controlExpression)
                .Accumulate(() => EvaluateExpressions(compilerInput, expressions))
                .Switch((evaluated, messages) =>
                       {
                           var (control, results) = evaluated;
                           LogMessages(messages);
                           foreach (var result in results)
                           {
                               CollectionAssert.AreEqual(control, result, comparer);
                           }
                       },
                       messages => ExpectingSuccess(messages, false));

        protected void AssertApproxEqual(CompilerInput compilerInput, string controlExpression, params float[][] results) =>
            AssertApproxEqual(compilerInput, controlExpression, FloatComparer, results);

        protected void AssertApproxEqual(CompilerInput compilerInput, string controlExpression, params string[] expressions)
            => AssertApproxEqual(compilerInput, controlExpression, FloatComparer, expressions);

        protected void AssertApproxEqualRelaxed(CompilerInput compilerInput, string controlExpression, params string[] expressions) 
            => AssertApproxEqual(compilerInput, controlExpression, RelaxedFloatComparer, expressions);

        protected void AssertTypeof(CompilerInput compilerInput, string expression, string typeStr) =>
            Host.Typeof(compilerInput, expression)
                .Switch((result, messages) =>
                {
                    LogMessages(messages);
                    Assert.That(result, Is.EqualTo(typeStr));
                }, messages => ExpectingSuccess(messages, false));
        
        protected void EvaluateExpectingElementError(CompilerInput compilerInput, EleMessageCode eleMessageCode, string expression)
        {
            var result = Host.Evaluate(compilerInput, expression);
            ExpectingError(result.Messages, result.IsSuccess, MessageExtensions.TypeString, (int)eleMessageCode);
        }
    }
}