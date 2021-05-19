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
        public void CallWithIncorrectArgCountFails(string expression) => EvaluateExpectingError(CompilerInput, ElementMessage.ArgumentCountMismatch, expression);

        [Test]
        public void RecursionDirectDisallowed() => EvaluateExpectingError(CompilerInput, ElementMessage.RecursionNotAllowed, "recurse(5)");
        [Test]
        public void RecursionIndirectDisallowed() => EvaluateExpectingError(CompilerInput, ElementMessage.RecursionNotAllowed, "recurseIndirect(5)");
        [Test]
        public void SelfReferencingLocalWithOuter() => EvaluateExpectingError(CompilerInput, ElementMessage.RecursionNotAllowed, "selfReferencingLocalWithOuter");
        [Test]
        public void SelfReferencingLocal() => EvaluateExpectingError(CompilerInput, ElementMessage.RecursionNotAllowed, "selfReferencingLocal");
        
        [TestCase("add", "IntrinsicFunction")]
        [TestCase("addThree", "ExpressionBodiedFunction")]
        [TestCase("addFive", "ScopeBodiedFunction")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilerInput, expression, type);
        
        [TestCase("add")]
        [TestCase("addThree")]
        [TestCase("addFive")]
        public void NotDeserializable(string expression) => EvaluateExpectingError(CompilerInput, ElementMessage.SerializationError, expression);

        [TestCase("5(10)")]
        [TestCase("pi(2)")]
        public void CallNonFunction(string expression) => EvaluateExpectingError(CompilerInput, ElementMessage.NotFunction, expression);
        
        [Test]
        public void LookupError() => EvaluateExpectingError(CompilerInput, ElementMessage.IdentifierNotFound, "funcWithLookupError");
    }
}