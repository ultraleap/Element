using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class HighOrderConstructs : SemanticsFixture
    {
        public HighOrderConstructs() : base("HighOrderFunctions") { }

        [Test]
        public void BindFunctionViaIndexing() => AssertTypeof(CompilationInput, "add", "Function");

        [Test]
        public void BindInstanceFunctionViaIndexing() => AssertTypeof(CompilationInput, "addFromInstanceFunction", "Function");

        [Test]
        public void CallFunctionWithFunctionResult() => AssertTypeof(CompilationInput, "getAdd(0)", "Function");

        [
            TestCase("returnsNumFunction(numFunctionInstance)", "Function", true),
            TestCase("returnsNumFunction(notNumFunctionInstance)", "Function", false),
            TestCase("returnsNumFunction(strictNumFunctionInstance)", "Function", true),
            TestCase("returnsNumFunction(strictFooFunctionInstance)", "Function", false),
        ] [
            TestCase("returnsFooFunction(numFunctionInstance)", "Function", false),
            TestCase("returnsFooFunction(notNumFunctionInstance)", "Function", true),
            TestCase("returnsFooFunction(strictNumFunctionInstance)", "Function", false),
            TestCase("returnsFooFunction(strictFooFunctionInstance)", "Function", true),
        ] [
            TestCase("returnsStrictNumFunction(numFunctionInstance)", "Function", false),
            TestCase("returnsStrictNumFunction(notNumFunctionInstance)", "Function", false),
            TestCase("returnsStrictNumFunction(strictNumFunctionInstance)", "Function", true),
            TestCase("returnsStrictNumFunction(strictFooFunctionInstance)", "Function", false),
        ] [
            TestCase("returnsStrictFooFunction(numFunctionInstance)", "Function", false),
            TestCase("returnsStrictFooFunction(notNumFunctionInstance)", "Function", false),
            TestCase("returnsStrictFooFunction(strictNumFunctionInstance)", "Function", false),
            TestCase("returnsStrictFooFunction(strictFooFunctionInstance)", "Function", true),
        ] [
            TestCase("applyNumFunction(numFunctionInstance, 5)", "Num", true),
            TestCase("applyNumFunction(numFunctionInstance, Foo(5))", "Num", false),
            TestCase("applyNumFunction(notNumFunctionInstance, 5)", "Num", false),
            TestCase("applyNumFunction(notNumFunctionInstance, Foo(5))", "Num", false),
            TestCase("applyNumFunction(strictNumFunctionInstance, 5)", "Num", true),
            TestCase("applyNumFunction(strictNumFunctionInstance, Foo(5))", "Num", false),
            TestCase("applyNumFunction(strictFooFunctionInstance, 5)", "Num", false),
            TestCase("applyNumFunction(strictFooFunctionInstance, Foo(5))", "Num", false),
        ] [
            TestCase("applyFooFunction(numFunctionInstance, 5)", "Foo", false),
            TestCase("applyFooFunction(numFunctionInstance, Foo(5))", "Foo", false),
            TestCase("applyFooFunction(notNumFunctionInstance, 5)", "Foo", false),
            TestCase("applyFooFunction(notNumFunctionInstance, Foo(5))", "Foo", true),
            TestCase("applyFooFunction(strictNumFunctionInstance, 5)", "Foo", false),
            TestCase("applyFooFunction(strictNumFunctionInstance, Foo(5))", "Foo", false),
            TestCase("applyFooFunction(strictFooFunctionInstance, 5)", "Foo", false),
            TestCase("applyFooFunction(strictFooFunctionInstance, Foo(5))", "Foo", true),
        ] [
            TestCase("applyStrictNumFunction(numFunctionInstance, 5)", "Num", false),
            TestCase("applyStrictNumFunction(numFunctionInstance, Foo(5))", "Num", false),
            TestCase("applyStrictNumFunction(notNumFunctionInstance, 5)", "Num", false),
            TestCase("applyStrictNumFunction(notNumFunctionInstance, Foo(5))", "Num", false),
            TestCase("applyStrictNumFunction(strictNumFunctionInstance, 5)", "Num", true),
            TestCase("applyStrictNumFunction(strictNumFunctionInstance, Foo(5))", "Num", false),
            TestCase("applyStrictNumFunction(strictFooFunctionInstance, 5)", "Num", false),
            TestCase("applyStrictNumFunction(strictFooFunctionInstance, Foo(5))", "Num", false),
        ] [
            TestCase("applyStrictFooFunction(numFunctionInstance, 5)", "Foo", false),
            TestCase("applyStrictFooFunction(numFunctionInstance, Foo(5))", "Foo", false),
            TestCase("applyStrictFooFunction(notNumFunctionInstance, 5)", "Foo", false),
            TestCase("applyStrictFooFunction(notNumFunctionInstance, Foo(5))", "Foo", false),
            TestCase("applyStrictFooFunction(strictNumFunctionInstance, 5)", "Foo", false),
            TestCase("applyStrictFooFunction(strictNumFunctionInstance, Foo(5))", "Foo", false),
            TestCase("applyStrictFooFunction(strictFooFunctionInstance, 5)", "Foo", false),
            TestCase("applyStrictFooFunction(strictFooFunctionInstance, Foo(5))", "Foo", true),
        ]
        public void ConstraintChecking(string expression, string type, bool succeeds)
        {
            if (succeeds) AssertTypeof(CompilationInput, expression, type);
            else EvaluateExpectingErrorCode(CompilationInput, 8, expression);
        }

        [Test]
        public void CaptureLifetimeExtendsForReturnedFunction() => AssertApproxEqual(CompilationInput, "addAndGetSub(5, 10)(20)", "-5");

        [Test]
        public void HighOrderFunctionSum() => AssertApproxEqual(CompilationInput, "sum(list(3, -5, 8, 20))", "26");
    }
}