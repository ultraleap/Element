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
        
        private Result<float[]> EvaluateControlExpression(CompilationInput compilationInput, string controlExpression) 
            => Host.Evaluate(compilationInput, controlExpression);

        private Result<float[][]> EvaluateExpressions(CompilationInput compilationInput, string[] expressions)
        {
            var resultBuilder = new ResultBuilder<float[][]>(new BasicTrace(compilationInput.Verbosity), new float[expressions.Length][]);

            for (var i = 0; i < expressions.Length; i++)
            {
                var expr = expressions[i];
                var exprResult = Host.Evaluate(compilationInput, expr);
                resultBuilder.Append(exprResult.Then(results =>
                {
                    // ReSharper disable once AccessToModifiedClosure
                    resultBuilder.Result[i] = results;
                }));
            }

            return resultBuilder.ToResult();
        }

        private void AssertApproxEqual(CompilationInput compilationInput, string controlExpression, IComparer comparer, params float[][] results) =>
            EvaluateControlExpression(compilationInput, controlExpression)
                .Switch((control, messages) =>
                       {
                           LogMessages(messages);
                           foreach (var result in results)
                           {
                               CollectionAssert.AreEqual(control, result, comparer);
                           }
                       },
                       messages => ExpectingSuccess(messages, false));

        private void AssertApproxEqual(CompilationInput compilationInput, string controlExpression, IComparer comparer, params string[] expressions) =>
            EvaluateControlExpression(compilationInput, controlExpression)
                .Accumulate(() => EvaluateExpressions(compilationInput, expressions))
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

        protected void AssertApproxEqual(CompilationInput compilationInput, string controlExpression, params float[][] results) =>
            AssertApproxEqual(compilationInput, controlExpression, FloatComparer, results);

        protected void AssertApproxEqual(CompilationInput compilationInput, string controlExpression, params string[] expressions)
            => AssertApproxEqual(compilationInput, controlExpression, FloatComparer, expressions);

        protected void AssertApproxEqualRelaxed(CompilationInput compilationInput, string controlExpression, params string[] expressions) 
            => AssertApproxEqual(compilationInput, controlExpression, RelaxedFloatComparer, expressions);

        protected void AssertTypeof(CompilationInput compilationInput, string expression, string typeStr) =>
            Host.Typeof(compilationInput, expression)
                .Switch((result, messages) =>
                {
                    LogMessages(messages);
                    Assert.That(result, Is.EqualTo(typeStr));
                }, messages => ExpectingSuccess(messages, false));

        protected static DirectoryInfo TestDirectory { get; } = new DirectoryInfo(Directory.GetCurrentDirectory());
        protected static FileInfo GetEleFile(string partialName) => GetFile("*.ele", partialName);
        protected static FileInfo GetFile(string pattern, string partialName) => TestDirectory.GetFiles(pattern, SearchOption.AllDirectories).Single(file => file.Name.Contains(partialName));


        protected void EvaluateExpectingErrorCode(CompilationInput compilationInput, MessageCode messageCode, string expression)
        {
            var result = Host.Evaluate(compilationInput, expression);
            ExpectingError(result.Messages, result.IsSuccess, messageCode);
        }
    }
}