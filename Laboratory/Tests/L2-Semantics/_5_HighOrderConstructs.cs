using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class _5_HighOrderConstructs : SemanticsFixture
    {
        public _5_HighOrderConstructs() : base("_5_HighOrderConstructs") { }

        [Test]
        public void BindFunctionViaIndexing() => AssertTypeof(CompilerInput, "add", "IntrinsicFunction");

        [Test]
        public void BindInstanceFunctionViaIndexing() => AssertTypeof(CompilerInput, "addFromInstanceFunction", "AppliedFunction");

        [Test]
        public void CallFunctionWithFunctionResult() => AssertTypeof(CompilerInput, "getAdd(0)", "AppliedFunction");

        [
            TestCase("returnsNumFunction(numFunctionInstance)", true, "ExpressionBodiedFunction"),
            TestCase("returnsNumFunction(notNumFunctionInstance)", false),
            TestCase("returnsNumFunction(strictNumFunctionInstance)", true, "ExpressionBodiedFunction"),
            TestCase("returnsNumFunction(strictFooFunctionInstance)", false),
        ] [
            TestCase("returnsFooFunction(numFunctionInstance)", false),
            TestCase("returnsFooFunction(notNumFunctionInstance)", true, "ExpressionBodiedFunction"),
            TestCase("returnsFooFunction(strictNumFunctionInstance)", false),
            TestCase("returnsFooFunction(strictFooFunctionInstance)", true, "ExpressionBodiedFunction"),
        ] [
            TestCase("returnsStrictNumFunction(numFunctionInstance)", false),
            TestCase("returnsStrictNumFunction(notNumFunctionInstance)", false),
            TestCase("returnsStrictNumFunction(strictNumFunctionInstance)", true, "ExpressionBodiedFunction"),
            TestCase("returnsStrictNumFunction(strictFooFunctionInstance)", false),
        ] [
            TestCase("returnsStrictFooFunction(numFunctionInstance)", false),
            TestCase("returnsStrictFooFunction(notNumFunctionInstance)", false),
            TestCase("returnsStrictFooFunction(strictNumFunctionInstance)", false),
            TestCase("returnsStrictFooFunction(strictFooFunctionInstance)", true, "ExpressionBodiedFunction"),
        ] [
            TestCase("applyNumFunction(numFunctionInstance, 5)", true, "Num"),
            TestCase("applyNumFunction(numFunctionInstance, Foo(5))", false),
            TestCase("applyNumFunction(notNumFunctionInstance, 5)", false),
            TestCase("applyNumFunction(notNumFunctionInstance, Foo(5))", false),
            TestCase("applyNumFunction(strictNumFunctionInstance, 5)", true, "Num"),
            TestCase("applyNumFunction(strictNumFunctionInstance, Foo(5))", false),
            TestCase("applyNumFunction(strictFooFunctionInstance, 5)", false),
            TestCase("applyNumFunction(strictFooFunctionInstance, Foo(5))", false),
        ] [
            TestCase("applyFooFunction(numFunctionInstance, 5)", false),
            TestCase("applyFooFunction(numFunctionInstance, Foo(5))", false),
            TestCase("applyFooFunction(notNumFunctionInstance, 5)", false),
            TestCase("applyFooFunction(notNumFunctionInstance, Foo(5))", true, "Foo"),
            TestCase("applyFooFunction(strictNumFunctionInstance, 5)", false),
            TestCase("applyFooFunction(strictNumFunctionInstance, Foo(5))", false),
            TestCase("applyFooFunction(strictFooFunctionInstance, 5)", false),
            TestCase("applyFooFunction(strictFooFunctionInstance, Foo(5))", true, "Foo"),
        ] [
            TestCase("applyStrictNumFunction(numFunctionInstance, 5)", false),
            TestCase("applyStrictNumFunction(numFunctionInstance, Foo(5))", false),
            TestCase("applyStrictNumFunction(notNumFunctionInstance, 5)", false),
            TestCase("applyStrictNumFunction(notNumFunctionInstance, Foo(5))", false),
            TestCase("applyStrictNumFunction(strictNumFunctionInstance, 5)", true, "Num"),
            TestCase("applyStrictNumFunction(strictNumFunctionInstance, Foo(5))", false),
            TestCase("applyStrictNumFunction(strictFooFunctionInstance, 5)", false),
            TestCase("applyStrictNumFunction(strictFooFunctionInstance, Foo(5))", false),
        ] [
            TestCase("applyStrictFooFunction(numFunctionInstance, 5)", false),
            TestCase("applyStrictFooFunction(numFunctionInstance, Foo(5))", false),
            TestCase("applyStrictFooFunction(notNumFunctionInstance, 5)", false),
            TestCase("applyStrictFooFunction(notNumFunctionInstance, Foo(5))", false),
            TestCase("applyStrictFooFunction(strictNumFunctionInstance, 5)", false),
            TestCase("applyStrictFooFunction(strictNumFunctionInstance, Foo(5))", false),
            TestCase("applyStrictFooFunction(strictFooFunctionInstance, 5)", false),
            TestCase("applyStrictFooFunction(strictFooFunctionInstance, Foo(5))", true, "Foo"),
        ]
        public void ConstraintChecking(string expression, bool succeeds, string type = default)
        {
            if (succeeds) AssertTypeof(CompilerInput, expression, type);
            else EvaluateExpectingElementError(CompilerInput, EleMessageCode.ConstraintNotSatisfied, expression);
        }

        [TestCase("lambdaChainConstant(0)(0)", "10")]
        [TestCase("lambdaChainConstantManyTimes(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)", "10")]
        public void LambdaChain(string expression, string expected) => AssertApproxEqual(CompilerInput, expression, expected);

        [Test]
        public void CaptureLifetimeExtendsForReturnedFunction() => AssertApproxEqual(CompilerInput, "addAndGetSub(5, 10)(20)", "-5");

        [Test]
        public void HighOrderFunctionSum() => AssertApproxEqual(CompilerInput, "sum(list(3, -5, 8, 20))", "26");
        
        [Test]
        public void ApplyHighOrderFunctionMultipleTimes() => AssertApproxEqual(CompilerInput, "usePartiallyAppliedFunctionMultipleTimes", "35"); 
        
        [TestCase("Indexer", "FunctionConstraint")]
        [TestCase("Binary", "FunctionConstraint")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilerInput, expression, type);
        
        [TestCase("Indexer")]
        [TestCase("Binary")]
        public void NotDeserializable(string expression) => EvaluateExpectingElementError(CompilerInput, EleMessageCode.SerializationError, expression);
        
        [TestCase("cyclicLocalFunction")]
        [TestCase("cyclicDefault")]
        public void RecursionDisallowed(string expression) => EvaluateExpectingElementError(CompilerInput, EleMessageCode.RecursionNotAllowed, expression);
    }
}