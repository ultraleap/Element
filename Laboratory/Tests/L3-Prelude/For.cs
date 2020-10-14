using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
    internal class For : PreludeFixture
    {
        [
            TestCase("for(5, _(n):Bool = True, _(n) = n)"),
            TestCase("for(False, _(n:Bool):Bool = True, _(n) = n)")
        ]
        public void GuaranteedInfiniteLoop(string expression) =>
            EvaluateExpectingElementError(ValidatedCompilerInput, EleMessageCode.InfiniteLoop, expression);

        public static (string FunctionExpression, string CallExpression, string ExpectedExpression)[] ImmediatelyTerminatingLoopData =
        {
            ("_(input:Num):Num = for(input, _(n):Bool = False, _(n) = n)", "(5)", "5"),
            ("_(input:Bool):Bool = for(input, _(n):Bool = False, _(n) = n)", "(True)", "True"),
            ("_(input:Vector2):Vector2 = for(input, _(n):Bool = False, _(n) = n)", "(Vector2(10, 20))", "Vector2(10, 20)"),
        };
        
        [Test]
        public void ImmediatelyTerminatingLoop([ValueSource(nameof(ImmediatelyTerminatingLoopData))] (string FunctionExpression, string CallExpression, string ExpectedExpression) args,
                                               [Values] EvaluationMode evaluationMode) =>
            AssertApproxEqual(ValidatedCompilerInput, args, evaluationMode);

        [Theory]
        public void ConstantExpressions((string ConstantExpression,string ExpectedExpression) args, EvaluationMode evaluationMode) =>
            AssertApproxEqual(ValidatedCompilerInput, args, evaluationMode);

        [DatapointSource]
        public (string ConstantExpression, string ExpectedExpression)[] ConstantExpressionData =
        {
            ("Loop.count(10, 2, _(s:Loop.IterationState) = s.state.add(5))", "20")
        };
        
        [Theory]
        public void FunctionCalls((string FunctionExpression, string CallExpression, string ExpectedExpression) args, EvaluationMode evaluationMode) =>
            AssertApproxEqual(ValidatedCompilerInput, args, evaluationMode);
        
        [DatapointSource]
        public (string FunctionExpression, string CallExpression, string ExpectedExpression)[] LoopFunctionCalls =
        {
            ("factorial", "(5)", "120"),
            ("NestedForLoop", "(0)", "200"),
            ("iterateWithMyStruct", "(5)", "5"),
        };
    }
}