using NUnit.Framework;

namespace Laboratory.Tests.Semantics
{
    internal class Constraints : SemanticsFixture
    {
        public Constraints() : base("Constraints") { }

        [TestCase("explicitAny(5)")]
        [TestCase("explicitAny(NotNum(5))")]
        [TestCase("explicitAny(a)")]
        public void ExplicitAny(string expression) => AssertApproxEqual(CompilationInput, expression, "5");

        [TestCase("onlyNum(5)", true)]
        [TestCase("onlyNum(NotNum(5))", false)]
        [TestCase("returnsNum(5)", true)]
        [TestCase("returnsNum(NotNum(5))", false)]
        [TestCase("returnsNotNum(5)", false)]
        [TestCase("returnsNotNum(NotNum(5))", true)]
        public void ConstraintChecking(string expression, bool succeeds)
        {
            if (succeeds) AssertApproxEqual(CompilationInput, expression, "5");
            else EvaluateExpectingErrorCode(CompilationInput, 8, expression);
        }

        [TestCase("explicitAny", "Function")]
        [TestCase("onlyNum", "Function")]
        [TestCase("returnsNum", "Function")]
        [TestCase("returnsNotNum", "Function")]
        [TestCase("NumFunction", "Constraint")]
        [TestCase("Any", "Constraint")]
        [TestCase("MySpace", "Namespace")]
        [TestCase("5", "Num")]
        [TestCase("a", "Num")]
        public void TypeofIs(string expression, string type) => AssertTypeof(CompilationInput, expression, type);

        [TestCase("MySpace")]
        [TestCase("NumFunction")]
        [TestCase("Num")]
        [TestCase("Any")]
        [TestCase("onlyNum")]
        public void NotDeserializable(string expression) => EvaluateExpectingErrorCode(CompilationInput, 1, expression);
    }
}