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

        [Test]
        public void TypeofFunction() => AssertTypeof(CompilationInput, "onlyNum", "Function");

        [TestCase("5"), TestCase("a")]
        public void TypeofNumber(string expression) => AssertTypeof(CompilationInput, expression, "Num");

        [Test]
        public void TypeofConstraint() => AssertTypeof(CompilationInput, "Any", "Constraint");

        [Test]
        public void TypeofNamespace() => AssertTypeof(CompilationInput, "MySpace", "Namespace");
    }
}