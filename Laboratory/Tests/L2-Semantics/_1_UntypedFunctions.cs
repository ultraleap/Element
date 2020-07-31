using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class _1_UntypedFunctions : SemanticsFixture
    {
        public _1_UntypedFunctions() : base("_1_UntypedFunctions") { }

        [TestCase("add(5, 8)", "13")]
        [TestCase("sub(3, 5.5)", "-2.5")]
        public void CallIntrinsic(string expression, string expectedResultExpression) => AssertApproxEqual(CompilationInput, expression, expectedResultExpression);
        
        [Test]
        public void CallWithIdentifierExpression() => AssertApproxEqual(CompilationInput, "add(pi, 6)",  "9.14");
        
        [Test]
        public void CallNestedCallExpression() => AssertApproxEqual(CompilationInput, "sub(add(pi, 2), 5)", "0.14");

        [Test]
        public void CallExpressionBodiedFunction() => AssertApproxEqual(CompilationInput, "addThree(-3.14, -9, pi)", "-9");

        [Test]
        public void CallScopeBodiedFunction() => AssertApproxEqual(CompilationInput, "addFive(-3.14, -9, pi, 29, 120)", "140");

        [TestCase("add(1)")]
        [TestCase("add(1, 3, 5)")]
        public void CallWithIncorrectArgCountFails(string expression) => EvaluateExpectingErrorCode(CompilationInput, MessageCode.ArgumentCountMismatch, expression);

        [Test]
        public void RecursionDirectDisallowed() => EvaluateExpectingErrorCode(CompilationInput, MessageCode.RecursionNotAllowed, "recurse(5)");
        [Test]
        public void RecursionIndirectDisallowed() => EvaluateExpectingErrorCode(CompilationInput, MessageCode.RecursionNotAllowed, "recurseIndirect(5)");
        
        [TestCase("add", "IntrinsicFunction")]
        [TestCase("addThree", "ExpressionBodiedFunction")]
        [TestCase("addFive", "ScopeBodiedFunction")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilationInput, expression, type);
        
        [TestCase("add")]
        [TestCase("addThree")]
        [TestCase("addFive")]
        public void NotDeserializable(string expression) => EvaluateExpectingErrorCode(CompilationInput, MessageCode.SerializationError, expression);

        [TestCase("5(10)")]
        [TestCase("pi(2)")]
        public void CallNonFunction(string expression) => EvaluateExpectingErrorCode(CompilationInput, MessageCode.NotFunction, expression);
    }
}