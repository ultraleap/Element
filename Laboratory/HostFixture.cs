using System;
using System.Collections;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Element;
using Element.NET.Tests;
using NUnit.Framework;

namespace Laboratory
{
    internal abstract class HostFixture : FixtureBase
    {
        protected readonly IHost Host = HostArguments.MakeHost();
        
        public const float FloatEpsilon = 1.19209e-5f;
        private static Comparer<float> FloatComparer { get; } = Comparer<float>.Create((f, f1) => ApproximatelyEqualEpsilon(f, f1, FloatEpsilon) ? 0 : 1);
        private static Comparer<float> RelaxedFloatComparer { get; } = Comparer<float>.Create((f, f1) => Math.Abs(f - f1) < FloatEpsilon ? 0 : 1);

        // Taken from https://stackoverflow.com/questions/3874627/floating-point-comparison-functions-for-c-sharp
        public static bool ApproximatelyEqualEpsilon(float a, float b, float epsilon)
        {
            const float floatNormal = (1 << 23) * float.Epsilon;
            float absA = Math.Abs(a);
            float absB = Math.Abs(b);
            float diff = Math.Abs(a - b);

            // ReSharper disable CompareOfFloatsByEqualityOperator
            if (a == b)
            {
                // Shortcut, handles infinities
                return true;
            }
            
            if (float.IsNaN(a) && float.IsNaN(b)) return true;

            if (a == 0.0f || b == 0.0f || diff < floatNormal)
            {    
                // a or b is zero, or both are extremely close to it.
                // relative error is less meaningful here
                return diff < (epsilon * floatNormal);
            }

            // use relative error
            return diff / Math.Min(absA + absB, float.MaxValue) < epsilon;
            // ReSharper enable CompareOfFloatsByEqualityOperator
        }
        
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

        protected static DirectoryInfo TestDirectory { get; } = new DirectoryInfo(Directory.GetCurrentDirectory());
        protected static FileInfo GetEleFile(string partialName) => GetFile("*.ele", partialName);
        protected static FileInfo GetFile(string pattern, string partialName) => TestDirectory.GetFiles(pattern, SearchOption.AllDirectories).Single(file => file.Name.Contains(partialName));


        protected void EvaluateExpectingErrorCode(CompilerInput compilerInput, MessageCode messageCode, string expression)
        {
            var result = Host.Evaluate(compilerInput, expression);
            ExpectingError(result.Messages, result.IsSuccess, messageCode);
        }
    }
}