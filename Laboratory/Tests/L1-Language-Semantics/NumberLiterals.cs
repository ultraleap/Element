using System.IO;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class NumberLiterals : HostFixture
    {
        public NumberLiterals(IHost host) : base(host) { }

        private static FileInfo[] SourceFiles => new []{GetTestFile("NumberLiterals")};

        private static CompilationInput CompilationInput => new CompilationInput(FailOnError, true, extraSourceFiles: SourceFiles);

        [Test]
        public void AIsPi() => AssertApproxEqual(CompilationInput, "a", 3.14f);
        
        [Test]
        public void BIsGoldenRatio() => AssertApproxEqual(CompilationInput, "b", 1.618f);
        
        [Test]
        public void CIsA() => AssertApproxEqual(CompilationInput, "c", "a");
        
        [Test]
        public void DIsB() => AssertApproxEqual(CompilationInput, "d", "b");

        [Test]
        public void EIsC() => AssertApproxEqual(CompilationInput, "e", "c");
    }
}