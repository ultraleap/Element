using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class _0_NumberLiterals : SemanticsFixture
    {
        public _0_NumberLiterals() : base("_0_NumberLiterals") { }

        [TestCase("a", "3.14")]
        [TestCase("b", "1.618")]
        [TestCase("c", "3.14")]
        [TestCase("d", "1.618")]
        [TestCase("e", "3.14")]
        public void Num(string expression, string expectedExpression) => AssertApproxEqual(CompilerInput, expression, expectedExpression);

        [Test]
        public void IdentifierNotFound() => EvaluateExpectingErrorCode(CompilerInput, MessageCode.IdentifierNotFound, "z");
        
        [TestCase("5", "Num")]
        [TestCase("a", "Num")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilerInput, expression, type);
    }
}