using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class Constraints : SemanticsFixture
    {
        public Constraints(IHost host) : base(host, "Constraints") { }

        [Test]
        public void ExplicitAnyAcceptsNum() => AssertApproxEqual(CompilationInput, "explicitAny(5)", "5");

        [Test]
        public void NumFailsGivenNonNum() => EvaluateExpectingErrorCode(CompilationInput, 8, "onlyNum(NotNum(5))");
    }
}