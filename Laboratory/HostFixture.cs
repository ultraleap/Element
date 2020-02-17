using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Element;
using NUnit.Framework;

namespace Laboratory
{
    [TestFixtureSource(typeof(HostArguments)), Parallelizable(ParallelScope.All)]
    internal abstract class HostFixture
    {
        protected HostFixture(IHost host)
        {
            _host = host;
        }

        protected readonly IHost _host;
        
        private const float FloatEpsilon = 1.19209e-5f;
        private static Comparer<float> FloatComparer { get; } = Comparer<float>.Create((f, f1) => ApproximatelyEqualEpsilon(f, f1, FloatEpsilon) ? 0 : 1);
        
        // Taken from https://stackoverflow.com/questions/3874627/floating-point-comparison-functions-for-c-sharp
        private static bool ApproximatelyEqualEpsilon(float a, float b, float epsilon)
        {
            const float floatNormal = (1 << 23) * float.Epsilon;
            float absA = Math.Abs(a);
            float absB = Math.Abs(b);
            float diff = Math.Abs(a - b);

            if (a == b)
            {
                // Shortcut, handles infinities
                return true;
            }

            if (a == 0.0f || b == 0.0f || diff < floatNormal)
            {    
                // a or b is zero, or both are extremely close to it.
                // relative error is less meaningful here
                return diff < (epsilon * floatNormal);
            }

            // use relative error
            return diff / Math.Min(absA + absB, float.MaxValue) < epsilon;
        }
      
        protected void AssertApproxEqual(CompilationInput compilationInput, string expression, float[] expected) =>
            CollectionAssert.AreEqual(_host.Evaluate(compilationInput, expression), expected, FloatComparer);
        protected void AssertApproxEqual(CompilationInput compilationInput, string expression, string otherExpression) =>
            CollectionAssert.AreEqual(_host.Evaluate(compilationInput, expression), _host.Evaluate(compilationInput, otherExpression), FloatComparer);

        protected void EvaluateExpectingErrorCode(CompilationInput compilationInput, int messageCode, string expression)
        {
            var withNewCallback = new CompilationInput(ExpectMessageCode(messageCode, compilationInput.LogCallback), compilationInput.ExcludePrelude, compilationInput.Packages, compilationInput.ExtraSourceFiles);
            _host.Evaluate(withNewCallback, expression);
            Assert.Fail("Expected message code '{0}' but no errors were logged", messageCode);
        }
        
        
        protected static DirectoryInfo TestDirectory { get; } = new DirectoryInfo(Directory.GetCurrentDirectory());
        protected static FileInfo GetTestFile(in string partialName) => TestDirectory.GetFiles($"*{partialName}*.*", SearchOption.AllDirectories).Single();
        protected static FileInfo[] GetTestFiles(in string pattern) => TestDirectory.GetFiles(pattern, SearchOption.AllDirectories);
        
        
        
        protected static void FailOnError(CompilerMessage message)
        {
            if (message.MessageLevel >= MessageLevel.Error)
                Assert.Fail(message.ToString());
            else
                TestContext.WriteLine(message.ToString());
        }

        protected static Action<CompilerMessage> ExpectMessageCode(int messageCode, Action<CompilerMessage> fallback) => message =>
        {
            if (message.MessageCode == messageCode) Assert.Pass($"Received expected message code {messageCode}");
            fallback?.Invoke(message);
        };
}
}