using System;
using System.Collections;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Element;
using NUnit.Framework;

namespace Laboratory
{
    [TestFixture, Parallelizable(ParallelScope.All)]
    internal abstract class HostFixture
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

        protected void AssertApproxEqual(CompilationInput compilationInput, string controlExpression, params string[] expressions) =>
            AssertApproxEqual(compilationInput, controlExpression, FloatComparer, expressions.Select(expression =>
            {
                var messages = new List<CompilerMessage>();
                compilationInput.LogCallback = CacheMessage(messages);
                var (success, result) = Host.Evaluate(compilationInput, expression);
                ExpectingSuccess(messages, success);
                
                return result;
            }).ToArray());

        protected void AssertApproxEqual(CompilationInput compilationInput, string controlExpression, params float[][] results)
        {
            AssertApproxEqual(compilationInput, controlExpression, FloatComparer, results);
        }
        
        private void AssertApproxEqual(CompilationInput compilationInput, string controlExpression, IComparer comparer, params float[][] results)
        {
            var messages = new List<CompilerMessage>();
            compilationInput.LogCallback = CacheMessage(messages);
            var (success, controlResult) = Host.Evaluate(compilationInput, controlExpression);
            ExpectingSuccess(messages, success);
            
            results.Aggregate(controlResult, (expected, result) =>
            {
                CollectionAssert.AreEqual(expected, result, comparer);
                return expected;
            });
        }
        
        
        
        protected void AssertApproxEqualRelaxed(CompilationInput compilationInput, string controlExpression, params string[] expressions) =>
            AssertApproxEqual(compilationInput, controlExpression, RelaxedFloatComparer, expressions.Select(expression =>
            {
                var messages = new List<CompilerMessage>();
                compilationInput.LogCallback = CacheMessage(messages);
                var (success, result) = Host.Evaluate(compilationInput, expression);
                ExpectingSuccess(messages, success);
                
                return result;
            }).ToArray());

        protected void AssertTypeof(CompilationInput compilationInput, string expression, string typeStr)
        {
            var (success, result) = Host.Typeof(compilationInput, expression);
            if (!success) 
                Assert.Fail();
            
            Assert.That(result, Is.EqualTo(typeStr));
        }

        protected static DirectoryInfo TestDirectory { get; } = new DirectoryInfo(Directory.GetCurrentDirectory());
        protected static FileInfo GetEleFile(string partialName) => GetFile("*.ele", partialName);
        protected static FileInfo GetFile(string pattern, string partialName) => TestDirectory.GetFiles(pattern, SearchOption.AllDirectories).Single(file => file.Name.Contains(partialName));


        protected void EvaluateExpectingErrorCode(CompilationInput compilationInput, int messageCode, string expression)
        {
            var messages = new List<CompilerMessage>();
            compilationInput.LogCallback = CacheMessage(messages);
            var (success, _) = Host.Evaluate(compilationInput, expression);

            ExpectingError(messages, success, messageCode);
        }
        
        protected static void LogMessage(CompilerMessage message)
        {
            TestContext.WriteLine(message.ToString());
            TestContext.WriteLine(string.Empty);
        }

        protected static Action<CompilerMessage> CacheMessage(List<CompilerMessage> messages) => message =>
        {
            messages.Add(message);
            LogMessage(message);
        };
        
        protected static void ExpectingError(List<CompilerMessage> messages, bool success, int messageCode)
        {
            var errors = messages.Where(s => s.MessageLevel >= MessageLevel.Error).ToArray();
            var hasErrors = errors.Any();
            
            if (messages.Any(s => s.MessageCode == messageCode)) 
                Assert.Pass($"Received expected message code {messageCode}");
            
            if (success) 
                Assert.Fail("Expected error ELE{0} '{1}' but succeeded",
                    messageCode, CompilerMessage.GetMessageName(messageCode));

            if (hasErrors) 
                Assert.Fail("Expected error ELE{0} '{1}' but got following error codes instead: {2}",
                    messageCode, CompilerMessage.GetMessageName(messageCode),
                    string.Join(", ", errors.Select(err => err.MessageCode)));
            
            Assert.Fail("Expected message code {0} '{1}', but success was false and no errors were received", messageCode,
                    CompilerMessage.GetMessageName(messageCode));
        }
        
        protected static void ExpectingSuccess(List<CompilerMessage> messages, bool success)
        {
            var errors = messages.Where(s => s.MessageLevel >= MessageLevel.Error).ToArray();
            var hasErrors = errors.Any();
            
            if (hasErrors) 
                Assert.Fail("Expected failure and got following code(s): {0}", 
                    string.Join(",", errors.Select(err => err.MessageCode)));
            
            if(success)
                Assert.Pass($"Received success");
            else
                Assert.Fail($"Received failure");
        }
    }
    
    
}