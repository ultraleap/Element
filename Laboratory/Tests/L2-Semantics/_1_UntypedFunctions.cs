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
        public void CallWithIncorrectArgCountFails(string expression) => EvaluateExpectingErrorCode(CompilerInput, MessageCode.ArgumentCountMismatch, expression);

        [Test]
        public void RecursionDirectDisallowed() => EvaluateExpectingErrorCode(CompilerInput, MessageCode.RecursionNotAllowed, "recurse(5)");
        [Test]
        public void RecursionIndirectDisallowed() => EvaluateExpectingErrorCode(CompilerInput, MessageCode.RecursionNotAllowed, "recurseIndirect(5)");
        [Test]
        public void SelfReferencingLocalWithOuter() => AssertApproxEqual(CompilerInput, "selfReferencingLocalWithOuter", "3.14");
        [Test]
        public void SelfReferencingLocal() => EvaluateExpectingErrorCode(CompilerInput, MessageCode.RecursionNotAllowed, "selfReferencingLocal");
        
        [TestCase("add", "IntrinsicFunction")]
        [TestCase("addThree", "ExpressionBodiedFunction")]
        [TestCase("addFive", "ScopeBodiedFunction")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilerInput, expression, type);
        
        [TestCase("add")]
        [TestCase("addThree")]
        [TestCase("addFive")]
        public void NotDeserializable(string expression) => EvaluateExpectingErrorCode(CompilerInput, MessageCode.SerializationError, expression);

        [TestCase("5(10)")]
        [TestCase("pi(2)")]
        public void CallNonFunction(string expression) => EvaluateExpectingErrorCode(CompilerInput, MessageCode.NotFunction, expression);
    }
}