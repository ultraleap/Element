using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class Constraints : SemanticsFixture
    {
        public Constraints(IHost host) : base(host, "Constraints") { }

        [TestCase("explicitAny(5)")]
        [TestCase("explicitAny(NotNum(5))")]
        [TestCase("explicitAny(a)")]
        public void ExplicitAny(string expression) => AssertApproxEqual(CompilationInput, expression, "5");

        [TestCase("onlyNum(NotNum(5))")]
        [TestCase("returnsNum(NotNum(5))")]
        [TestCase("returnsNotNum(5)")]
        public void ConstraintNotSatisfied(string expression) => EvaluateExpectingErrorCode(CompilationInput, 8, expression);

        [TestCase("explicitAny", "Function")]
        [TestCase("onlyNum", "Function")]
        [TestCase("returnsNum", "Function")]
        [TestCase("returnsNotNum", "Function")]
        [TestCase("Predicate", "Constraint")]
        [TestCase("Any", "Constraint")]
        [TestCase("MySpace", "Namespace")]
        [TestCase("5", "Num")]
        [TestCase("a", "Num")]
        public void TypeofIs(string expression, string type) => AssertTypeof(CompilationInput, expression, type);

        [TestCase("MySpace")]
        [TestCase("Predicate")]
        [TestCase("Num")]
        [TestCase("Any")]
        [TestCase("onlyNum")]
        public void NotDeserializable(string expression) => EvaluateExpectingErrorCode(CompilationInput, 1, expression);
    }
}