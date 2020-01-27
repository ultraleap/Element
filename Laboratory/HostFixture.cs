using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Element;
using NUnit.Framework;
using NUnit.Framework.Constraints;

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

        
        
        private const float FloatEpsilon = 1.19209e-7f;
        protected static EqualConstraint FloatIsApproximately(in object expected) => Is.EqualTo(expected).Within(FloatEpsilon);
        protected static Comparer<float> FloatComparer { get; } = Comparer<float>.Create((f, f1) => MathF.Abs(f - f1) <= FloatEpsilon ? 0 : 1);

        
        
        protected float EvaluateConstant(in CompilationInput compilationInput, in string constantIdentifier) => _host.Execute(compilationInput, constantIdentifier).Single();
        protected float[] EvaluateCall(in CompilationInput compilationInput, in string identifier, params float[] arguments) => _host.Execute(compilationInput, identifier, arguments);
        
        
        
        protected void AssertApproxEqual(in CompilationInput compilationInput, in string constantIdentifier, in float constant) =>
            Assert.That(EvaluateConstant(compilationInput, constantIdentifier), FloatIsApproximately(constant));
        protected void AssertApproxEqual(in CompilationInput compilationInput, in string constantIdentifier, in string otherConstantIdentifer) =>
            Assert.That(EvaluateConstant(compilationInput, constantIdentifier), FloatIsApproximately(EvaluateConstant(compilationInput, otherConstantIdentifer)));
        protected void AssertApproxEqual(in CompilationInput compilationInput, in string identifier, in float[] arguments, in float constant) =>
            Assert.That(EvaluateCall(compilationInput, identifier, arguments).Single(), FloatIsApproximately(constant));
        protected void AssertApproxEqual(in CompilationInput compilationInput, in string identifier, in float[] arguments, in float[] expected) =>
            CollectionAssert.AreEqual(EvaluateCall(compilationInput, identifier, arguments), expected, FloatComparer);
        
        
        
        protected static DirectoryInfo TestDirectory { get; } = new DirectoryInfo(Directory.GetCurrentDirectory());
        protected static FileInfo GetTestFile(in string partialName) => TestDirectory.GetFiles($"*{partialName}*.*", SearchOption.AllDirectories).Single();
        protected static FileInfo[] GetTestFiles(in string pattern) => TestDirectory.GetFiles(pattern, SearchOption.AllDirectories);
        
        
        
        protected static void FailOnError(CompilerMessage message)
        {
            if (message.MessageLevel >= MessageLevel.Error)
                Assert.Fail(message.ToString());
        }

        protected static Action<CompilerMessage> ExpectMessageCode(int messageCode) => message =>
        {
            if (message.MessageCode == messageCode) Assert.Pass($"Received expected message code {messageCode}");
            FailOnError(message);
        };
}
}