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
        public void CaptureLifetimeExtendsForReturnedFunction() => AssertApproxEqual(CompilationInput, "addAndGetSub(5, 10)(20)", "-5");

        [Test]
        public void HighOrderFunctionSum() => AssertApproxEqual(CompilationInput, "sum(list(3, -5, 8, 20))", "26");
    }
}