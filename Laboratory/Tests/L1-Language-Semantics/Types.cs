using System.IO;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class Types : HostFixture
    {
        public Types(IHost host) : base(host) { }
        
        private static FileInfo[] SourceFiles => new []{GetTestFile("Types")};

        private static CompilationInput CompilationInput => new CompilationInput(FailOnError, true, extraSourceFiles: SourceFiles);

        [Test]
        public void ExplicitAnyAcceptsNum() => AssertApproxEqual(CompilationInput, "explicitAny(5)", "5");

        [Test]
        public void NumFailsGivenNonNum() => EvaluateExpectingErrorCode(CompilationInput, 8, "onlyNum(NotNum(5))");
        
        [Test]
        public void NoImplicitConversionFromAliasToAliased() => EvaluateExpectingErrorCode(CompilationInput, 8, "onlyNum(NumAlias(5))");

        [Test]
        public void NoImplicitConversionFromAliasedToAlias() => EvaluateExpectingErrorCode(CompilationInput, 8, "onlyNumAlias(5)");

        [Test]
        public void AliasFunctionAcceptsAlias() => AssertApproxEqual(CompilationInput, "onlyNumAlias(NumAlias(9))", "9");
    }
}