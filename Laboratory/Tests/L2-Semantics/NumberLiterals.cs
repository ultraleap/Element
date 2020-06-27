using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class NumberLiterals : SemanticsFixture
    {
        public NumberLiterals() : base("NumberLiterals") { }

        [Test]
        public void AIsPi() => AssertApproxEqual(CompilationInput, "a", "3.14");
        
        [Test]
        public void BIsGoldenRatio() => AssertApproxEqual(CompilationInput, "b", "1.618");
        
        [Test]
        public void CIsA() => AssertApproxEqual(CompilationInput, "c", "a");
        
        [Test]
        public void DIsB() => AssertApproxEqual(CompilationInput, "d", "b");

        [Test]
        public void EIsC() => AssertApproxEqual(CompilationInput, "e", "c");

        [Test]
        public void RecursionDisallowed() => EvaluateExpectingErrorCode(CompilationInput, MessageCode.CircularCompilation, "f");
    }
}