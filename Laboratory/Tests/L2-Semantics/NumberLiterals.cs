using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class NumberLiterals : SemanticsFixture
    {
        public NumberLiterals(IHost host) : base(host, "NumberLiterals") { }

        [Test]
        public void AIsPi() => AssertApproxEqual(CompilationInput, "a", new[]{3.14f});
        
        [Test]
        public void BIsGoldenRatio() => AssertApproxEqual(CompilationInput, "b", new[]{1.618f});
        
        [Test]
        public void CIsA() => AssertApproxEqual(CompilationInput, "c", "a");
        
        [Test]
        public void DIsB() => AssertApproxEqual(CompilationInput, "d", "b");

        [Test]
        public void EIsC() => AssertApproxEqual(CompilationInput, "e", "c");
    }
}