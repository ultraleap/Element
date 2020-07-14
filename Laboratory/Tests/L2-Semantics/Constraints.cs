using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
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
            else EvaluateExpectingErrorCode(CompilationInput, MessageCode.ConstraintNotSatisfied, expression);
        }

        [TestCase("explicitAny", "explicitAny:ExpressionBodiedFunction")]
        [TestCase("onlyNum", "onlyNum:ExpressionBodiedFunction")]
        [TestCase("returnsNum", "returnsNum:ExpressionBodiedFunction")]
        [TestCase("returnsNotNum", "returnsNotNum:ScopeBodiedFunction")]
        
        [TestCase("NumFunction", "NumFunction:FunctionConstraint")]
        [TestCase("Any", "Any:IntrinsicConstraint")]
        
        [TestCase("MySpace", "MySpace:Namespace")]
        
        [TestCase("5", "5")]
        [TestCase("a", "Num")] // TODO: Fails because it's not possible to prevent full resolution of the expr!
        public void TypeofIs(string expression, string type) => AssertTypeof(CompilationInput, expression, type);

        [TestCase("MySpace")]
        [TestCase("NumFunction")]
        [TestCase("Num")]
        [TestCase("Any")]
        [TestCase("onlyNum")]
        public void NotDeserializable(string expression) => EvaluateExpectingErrorCode(CompilationInput, MessageCode.SerializationError, expression);
    }
}