using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class _2_TypedFunctions : SemanticsFixture
    {
        public _2_TypedFunctions() : base("_2_TypedFunctions") { }

        [TestCase("explicitAny(5)", true)]
        [TestCase("explicitAny(NotNum(5))", true)]
        [TestCase("explicitAny(a)", true)]
        [TestCase("explicitAny(b)", true)]
        [TestCase("onlyNum(5)", true)]
        [TestCase("onlyNum(NotNum(5))", false)]
        [TestCase("onlyNum(a)", true)]
        [TestCase("onlyNum(b)", false)]
        public void CallWithInputConstraints(string expression, bool succeeds)
        {
            if (succeeds) AssertApproxEqual(CompilerInput, expression, "5");
            else EvaluateExpectingElementError(CompilerInput, EleMessageCode.ConstraintNotSatisfied, expression);
        }
        
        [TestCase("returnsNum(5)", true)]
        [TestCase("returnsNum(NotNum(5))", false)]
        [TestCase("returnsNotNum(5)", false)]
        [TestCase("returnsNotNum(NotNum(5))", true)]
        public void CallWithReturnConstraint(string expression, bool succeeds)
        {
            if (succeeds) AssertApproxEqual(CompilerInput, expression, "5");
            else EvaluateExpectingElementError(CompilerInput, EleMessageCode.ConstraintNotSatisfied, expression);
        }
        
        [TestCase("Any", "IntrinsicConstraint")]
        [TestCase("Num", "IntrinsicStruct")]
        [TestCase("NotNum", "CustomStruct")]
        [TestCase("b", "NotNum")]
        [TestCase("explicitAny", "ExpressionBodiedFunction")]
        [TestCase("onlyNum", "ExpressionBodiedFunction")]
        [TestCase("returnsNum", "ExpressionBodiedFunction")]
        [TestCase("returnsNotNum", "ScopeBodiedFunction")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilerInput, expression, type);
        
        [TestCase("Any")]
        [TestCase("Num")]
        [TestCase("NotNum")]
        public void NotDeserializable(string expression) => EvaluateExpectingElementError(CompilerInput, EleMessageCode.SerializationError, expression);

        [TestCase("literalNumNotAConstraint(5)")]
        [TestCase("literalFunctionNotAConstraint(5)")]
        public void ConstraintWithNonConstraint(string expression) => EvaluateExpectingElementError(CompilerInput, EleMessageCode.NotConstraint, expression);

        [TestCase("Any(20)")]
        public void CallNonFunction(string expression) => EvaluateExpectingElementError(CompilerInput, EleMessageCode.NotFunction, expression);
    }
}