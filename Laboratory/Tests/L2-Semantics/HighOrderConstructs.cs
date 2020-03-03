using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class HighOrderConstructs : SemanticsFixture
    {
        public HighOrderConstructs(IHost host) : base(host, "HighOrderConstructs") { }

        [Test]
        public void BindFunctionViaIndexing() => AssertTypeof(CompilationInput, "add", "Function");

        [Test]
        public void BindInstanceFunctionViaIndexing() => AssertTypeof(CompilationInput, "addFromInstanceFunction", "Function");

        [Test]
        public void CallFunctionWithFunctionResult() => AssertTypeof(CompilationInput, "getAdd(0)", "Function");

        [Test]
        public void CallFunctionWithStructResult() => AssertTypeof(CompilationInput, "voldemort(5)", "Type");

        [Test]
        public void HighOrderFunctionSum() => AssertTypeof(CompilationInput, "sum(list(3, -5, 8, 20))", "26");

        [Test]
        public void RecursionDisallowed() => EvaluateExpectingErrorCode(CompilationInput, 11, "recursiveSum(10)");
    }
}