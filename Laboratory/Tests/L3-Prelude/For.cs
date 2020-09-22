using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
    internal class For : PreludeFixture
    {
        [Test]
        public void GuaranteedInfiniteLoop() =>
            EvaluateExpectingElementError(ValidatedCompilerInput, EleMessageCode.InfiniteLoop, "for(5, _(n):Bool = True, _(n) = n)");

        [TestCase("5")]
        [TestCase("Vector2(10, 20)")]
        public void ImmediatelyTerminatingLoop(string loopInitial) =>
            AssertApproxEqual(ValidatedCompilerInput, $"for({loopInitial}, _(n):Bool = False, _(n) = n)", loopInitial);

        [TestCase("Loop.count(10, 2, _(s:Loop.IterationState) = s.state.add(5))", "20")]
        public void CompileTimeConstantLoop(string expr, string result) => AssertApproxEqual(ValidatedCompilerInput, expr, result);
        
        [TestCase("iterateWithMyStruct(5)", "5")]
        public void LoopReturningFor(string expr, string result) => AssertApproxEqual(ValidatedCompilerInput, expr, result);

        [TestCase("NestedForLoop(0)", "200")]
        public void NestedLoop(string expr, string result) => AssertApproxEqual(ValidatedCompilerInput, expr, result);
        
        [TestCase("factorial", "(5)", "120")]
        public void NonConstantLoop(string expr, string callExpr, string expectedResult) =>
            AssertApproxEqual(ValidatedCompilerInput, new FunctionEvaluation(expr, callExpr, false), expectedResult);
    }
}