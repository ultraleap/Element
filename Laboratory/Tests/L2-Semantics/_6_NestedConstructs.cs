using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class _6_NestedConstructs : SemanticsFixture
    {
        public _6_NestedConstructs() : base("_6_NestedConstructs") { }

        [Test]
        public void AddUsingLocal() => AssertApproxEqual(CompilerInput, "12", "addUsingLocal(6, 6)");

        [Test]
        public void AddUsingLocalWithCaptures() => AssertApproxEqual(CompilerInput, "12", "addUsingLocalCapture(6, 6)");
        
        [Test]
        public void AddUsingLocalWithCapturesAsFirstClassFunction() => AssertApproxEqual(CompilerInput, "12", "addUsingLocalCaptureAsFirstClassFunction(6, 6)");

        [Test]
        public void AddUsingLocalWithCapturesCalledMultipleTimes() => AssertApproxEqual(CompilerInput, "18", "addBTwice(6, 6)");

        [Test]
        public void AddUsingLocalWithShadowing() => AssertApproxEqual(CompilerInput, "12", "addUsingLocalWithShadowing(6, 6)");

        [Test]
        public void LocalStructInstance() => AssertTypeof(CompilerInput, "returnLocalStructInstance(5)", "Vector2");

        [Test]
        public void LocalStructInstanceFunction() => AssertApproxEqual(CompilerInput, "15", "returnLocalStructInstance(5).add(10).x");

        [Test]
        public void ClosureWithinAnonymousFunction() => AssertApproxEqual(CompilerInput, "13", "addUsingLambdaWithCapture(5, 8)");
        
        [Test]
        public void DeepClosureWithinAnonymousFunction() => AssertApproxEqual(CompilerInput, "19", "deepNestedLambdaWithCapture(5, 8, 6)");
        
        [Test]
        public void AddUsingDeepNestedCapture() => AssertApproxEqual(CompilerInput, "13", "addUsingDeepNestedCapture(5, 8)");
        
        [Test]
        public void AddUsingDeepNestedCaptureWithLambda() => AssertApproxEqual(CompilerInput, "13", "addUsingDeepNestedCaptureWithLambda(5, 8)");
        
        [Test]
        public void NestedNamespacesInStructBody() => AssertTypeof(CompilerInput, "rootStruct.nestedNamespace", "Namespace");

        [Test]
        public void LocalFunctionConstraintNotFound() => EvaluateExpectingError(CompilerInput, ElementMessage.IdentifierNotFound, "localFuncWithConstraintError");
        [Test]
        public void LocalFunctionConstraintNotFoundDoesntContinueLookup() => EvaluateExpectingError(CompilerInput, ElementMessage.IdentifierNotFound, "localFuncWithErrorThatShadowsOuterId");
    }
}