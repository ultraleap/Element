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

        [Theory]
        public void ReturnScopeBodiedLocal(EvaluationMode mode) => AssertTypeof(CompilerInput, new ExpressionEvaluation("scopeBodiedLocal(5)", mode), "ScopeBodiedFunction");

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
            else EvaluateExpectingError(CompilerInput, ElementMessage.ConstraintNotSatisfied, expression);
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

        [Test]
        public void NestedFunctionReturnWithIncorrectSignatureReturnsError() => EvaluateExpectingError(CompilerInput, ElementMessage.ConstraintNotSatisfied, "innerFuncSignatureIncorrect(10)");
        
        [TestCase("Indexer", "FunctionConstraint")]
        [TestCase("Binary", "FunctionConstraint")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilerInput, expression, type);
        
        [TestCase("Indexer")]
        [TestCase("Binary")]
        public void NotDeserializable(string expression) => EvaluateExpectingError(CompilerInput, ElementMessage.SerializationError, expression);
        
        [TestCase("cyclicLocalFunction")]
        [TestCase("cyclicDefault")]
        public void RecursionDisallowed(string expression) => EvaluateExpectingError(CompilerInput, ElementMessage.RecursionNotAllowed, expression);
        
        [Test]
        public void CanResolveTupleWithSameDeclarationResolvedMultipleTimes() => AssertApproxEqual(CompilerInput, "getTuple.i", "2");

        [Test]
        public void VectorsShouldBeFullyResolvedBeforeSuccessiveCall() => AssertApproxEqual(CompilerInput, "getVec.i", "2");
        
        //[Test]
        //public void StructsShouldBeFullyResolvedBeforeSuccessiveCall() => AssertApproxEqual(CompilerInput, "getStruct.i", "2");

        [Theory]
        public void TripleNestedHighOrderFunction(EvaluationMode mode) => AssertApproxEqual(CompilerInput, ("numRenderer(5)(addFiveEvaluator)(0).at(0)", "10"), mode);
        
        public static (string constructorArgs, string fName, string constant)[] RecursionErrorArgs =
        {
            ("(0)", "wrappedLambdasOneLine", "0"),
            ("(0)", "wrappedLambdasExplicit", "0"),
            ("(0)", "wrappedLambdasNamed", "0"),
            ("(0)", "adderRecursionLambdas", "10"),
            ("(0)", "adderRecursionReturnFunction", "10"),
        };
        [Test]
        public void DistinctUsagesOfSameDeclarationShouldNotBeDetectedAsRecursion([ValueSource(nameof(RecursionErrorArgs))] (string lhs, string fName, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(u:Num):Num = " + args.fName + "(u)";
            AssertApproxEqual(CompilerInput,
                              new FunctionEvaluation(testFunction, args.lhs, mode),
                              new ExpressionEvaluation(args.rhs, mode));
        }
        
        [Test]
        public void ConstraintError([Values] EvaluationMode mode)
        {
            string testFunction = "_(u:Num):Num = constraintError(u)";
            string args = "(0)";
            string expected = "0";
            AssertApproxEqual(CompilerInput,
                              new FunctionEvaluation(testFunction, args, mode),
                              new ExpressionEvaluation(expected, mode));
        }
    }
}