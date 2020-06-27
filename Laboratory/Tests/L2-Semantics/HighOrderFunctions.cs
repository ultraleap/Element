using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class HighOrderConstructs : SemanticsFixture
    {
        public HighOrderConstructs() : base("HighOrderFunctions") { }

        [Test]
        public void BindFunctionViaIndexing() => AssertTypeof(CompilationInput, "add", "Num.add:Function");

        [Test]
        public void BindInstanceFunctionViaIndexing() => AssertTypeof(CompilationInput, "addFromInstanceFunction", "Num.add:Function");

        [Test]
        public void CallFunctionWithFunctionResult() => AssertTypeof(CompilationInput, "getAdd(0)", "Num.add:Function");

        [
            TestCase("returnsNumFunction(numFunctionInstance)", true, "numFunctionInstance:Function"),
            TestCase("returnsNumFunction(notNumFunctionInstance)", false),
            TestCase("returnsNumFunction(strictNumFunctionInstance)", true, "strictNumFunctionInstance:Function"),
            TestCase("returnsNumFunction(strictFooFunctionInstance)", false),
        ] [
            TestCase("returnsFooFunction(numFunctionInstance)", false),
            TestCase("returnsFooFunction(notNumFunctionInstance)", true, "notNumFunctionInstance:Function"),
            TestCase("returnsFooFunction(strictNumFunctionInstance)", false),
            TestCase("returnsFooFunction(strictFooFunctionInstance)", true, "strictFooFunctionInstance:Function"),
        ] [
            TestCase("returnsStrictNumFunction(numFunctionInstance)", false),
            TestCase("returnsStrictNumFunction(notNumFunctionInstance)", false),
            TestCase("returnsStrictNumFunction(strictNumFunctionInstance)", true, "strictNumFunctionInstance:Function"),
            TestCase("returnsStrictNumFunction(strictFooFunctionInstance)", false),
        ] [
            TestCase("returnsStrictFooFunction(numFunctionInstance)", false),
            TestCase("returnsStrictFooFunction(notNumFunctionInstance)", false),
            TestCase("returnsStrictFooFunction(strictNumFunctionInstance)", false),
            TestCase("returnsStrictFooFunction(strictFooFunctionInstance)", true, "strictFooFunctionInstance:Function"),
        ] [
            TestCase("applyNumFunction(numFunctionInstance, 5)", true, "5"),
            TestCase("applyNumFunction(numFunctionInstance, Foo(5))", false),
            TestCase("applyNumFunction(notNumFunctionInstance, 5)", false),
            TestCase("applyNumFunction(notNumFunctionInstance, Foo(5))", false),
            TestCase("applyNumFunction(strictNumFunctionInstance, 5)", true, "5"),
            TestCase("applyNumFunction(strictNumFunctionInstance, Foo(5))", false),
            TestCase("applyNumFunction(strictFooFunctionInstance, 5)", false),
            TestCase("applyNumFunction(strictFooFunctionInstance, Foo(5))", false),
        ] [
            TestCase("applyFooFunction(numFunctionInstance, 5)", false),
            TestCase("applyFooFunction(numFunctionInstance, Foo(5))", false),
            TestCase("applyFooFunction(notNumFunctionInstance, 5)", false),
            TestCase("applyFooFunction(notNumFunctionInstance, Foo(5))", true, "Instance:Foo:Struct"),
            TestCase("applyFooFunction(strictNumFunctionInstance, 5)", false),
            TestCase("applyFooFunction(strictNumFunctionInstance, Foo(5))", false),
            TestCase("applyFooFunction(strictFooFunctionInstance, 5)", false),
            TestCase("applyFooFunction(strictFooFunctionInstance, Foo(5))", true, "Instance:Foo:Struct"),
        ] [
            TestCase("applyStrictNumFunction(numFunctionInstance, 5)", false),
            TestCase("applyStrictNumFunction(numFunctionInstance, Foo(5))", false),
            TestCase("applyStrictNumFunction(notNumFunctionInstance, 5)", false),
            TestCase("applyStrictNumFunction(notNumFunctionInstance, Foo(5))", false),
            TestCase("applyStrictNumFunction(strictNumFunctionInstance, 5)", true, "5"),
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
            TestCase("applyStrictFooFunction(strictFooFunctionInstance, Foo(5))", true, "Instance:Foo:Struct"),
        ]
        public void ConstraintChecking(string expression, bool succeeds, string type = default)
        {
            if (succeeds) AssertTypeof(CompilationInput, expression, type);
            else EvaluateExpectingErrorCode(CompilationInput, MessageCode.ConstraintNotSatisfied, expression);
        }

        [Test]
        public void CaptureLifetimeExtendsForReturnedFunction() => AssertApproxEqual(CompilationInput, "addAndGetSub(5, 10)(20)", "-5");

        [Test]
        public void HighOrderFunctionSum() => AssertApproxEqual(CompilationInput, "sum(list(3, -5, 8, 20))", "26");
    }
}