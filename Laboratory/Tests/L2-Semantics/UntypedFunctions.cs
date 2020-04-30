using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class UntypedFunctions : SemanticsFixture
    {
        public UntypedFunctions() : base("UntypedFunctions") { }

        [Test]
        public void Add() => AssertApproxEqual(CompilationInput, "Num.add(5, 8)", "13");
        
        [Test]
        public void Sub() => AssertApproxEqual(CompilationInput, "Num.sub(3, 5.5)", "-2.5");
        
        [Test]
        public void AddWithIdentifierExpression() => AssertApproxEqual(CompilationInput, "Num.add(pi, 6)",  "9.14");
        
        [Test]
        public void NestedCallExpression() => AssertApproxEqual(CompilationInput, "Num.sub(Num.add(pi, 2), 5)", "0.14");

        [Test]
        public void CustomExpressionBodiedFunctionCall() => AssertApproxEqual(CompilationInput, "addThree(-3.14, -9, pi)", "-9");

        [Test]
        public void CustomScopeBodiedFunctionCall() => AssertApproxEqual(CompilationInput, "addFive(-3.14, -9, pi, 29, 120)", "140");

        [TestCase("Num.add(1)")]
        [TestCase("Num.add(1, 3, 5)")]
        public void IncorrectArgCountFails(string expression) => EvaluateExpectingErrorCode(CompilationInput, 6, expression);

        [Test]
        public void RecursionDisallowed() => EvaluateExpectingErrorCode(CompilationInput, 11, "recurse(5)");
    }
}