using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
    internal class For : PreludeFixture
    {
        [Test]
        public void GuaranteedInfiniteLoop() =>
            EvaluateExpectingErrorCode(ValidatedCompilationInput, MessageCode.InfiniteLoop, "for(5, _(n):Bool = True, _(n) = n)");

        [TestCase("5")]
        [TestCase("Vector2(10, 20)")]
        public void ImmediatelyTerminatingLoop(string loopInitial) =>
            AssertApproxEqual(ValidatedCompilationInput, $"for({loopInitial}, _(n):Bool = False, _(n) = n)", loopInitial);

        [TestCase("Loop.count(10, 2, _(s:Loop.IterationState) = s.state.add(5))", "20")]
        public void CompileTimeConstantLoop(string expr, string result) => AssertApproxEqual(ValidatedCompilationInput, expr, result);
        
        // TODO: Somehow test non-constant loops, how can we define a non-constant loop as an expression here? 

        // TODO: Nested loop test
        //public void NestedLoop(string expr, string result) => AssertApproxEqual(ValidatedCompilationInput, expr, result);
    }
}