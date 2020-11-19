using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class _1_UntypedFunctions : SemanticsFixture
    {
        public _1_UntypedFunctions() : base("_1_UntypedFunctions") { }

        [TestCase("add(5, 8)", "13")]
        [TestCase("sub(3, 5.5)", "-2.5")]
        public void CallIntrinsic(string expression, string expectedResultExpression) => AssertApproxEqual(CompilerInput, expression, expectedResultExpression);
        
        [Test]
        public void CallWithIdentifierExpression() => AssertApproxEqual(CompilerInput, "add(pi, 6)",  "9.14");
        
        [Test]
        public void CallNestedCallExpression() => AssertApproxEqual(CompilerInput, "sub(add(pi, 2), 5)", "0.14");

        [Test]
        public void CallExpressionBodiedFunction() => AssertApproxEqual(CompilerInput, "addThree(-3.14, -9, pi)", "-9");

        [Test]
        public void CallScopeBodiedFunction() => AssertApproxEqual(CompilerInput, "addFive(-3.14, -9, pi, 29, 120)", "140");

        [TestCase("add(1)")]
        [TestCase("add(1, 3, 5)")]
        public void CallWithIncorrectArgCountFails(string expression) => EvaluateExpectingElementError(CompilerInput, EleMessageCode.ArgumentCountMismatch, expression);

        [Test]
        public void RecursionDirectDisallowed() => EvaluateExpectingElementError(CompilerInput, EleMessageCode.RecursionNotAllowed, "recurse(5)");
        [Test]
        public void RecursionIndirectDisallowed() => EvaluateExpectingElementError(CompilerInput, EleMessageCode.RecursionNotAllowed, "recurseIndirect(5)");
        [Test]
        public void SelfReferencingLocalWithOuter() => EvaluateExpectingElementError(CompilerInput, EleMessageCode.RecursionNotAllowed, "selfReferencingLocalWithOuter");
        [Test]
        public void SelfReferencingLocal() => EvaluateExpectingElementError(CompilerInput, EleMessageCode.RecursionNotAllowed, "selfReferencingLocal");
        
        [TestCase("add", "IntrinsicFunction")]
        [TestCase("addThree", "ExpressionBodiedFunction")]
        [TestCase("addFive", "ScopeBodiedFunction")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilerInput, expression, type);
        
        [TestCase("add")]
        [TestCase("addThree")]
        [TestCase("addFive")]
        public void NotDeserializable(string expression) => EvaluateExpectingElementError(CompilerInput, EleMessageCode.SerializationError, expression);

        [TestCase("5(10)")]
        [TestCase("pi(2)")]
        public void CallNonFunction(string expression) => EvaluateExpectingElementError(CompilerInput, EleMessageCode.NotFunction, expression);
        
        [Test]
        public void LookupError() => EvaluateExpectingElementError(CompilerInput, EleMessageCode.IdentifierNotFound, "funcWithLookupError");
    }
}