using System.IO;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class UntypedFunctions : HostFixture
    {
        public UntypedFunctions(IHost host) : base(host) { }

        private static FileInfo[] SourceFiles => new []{GetTestFile("UntypedFunctions")};

        private static CompilationInput CompilationInput => new CompilationInput(FailOnError, true, extraSourceFiles: SourceFiles);


        [Test]
        public void Add() => AssertApproxEqual(CompilationInput, "Num.add(5, 8)", new []{13f});
        
        [Test]
        public void Sub() => AssertApproxEqual(CompilationInput, "Num.sub(3, 5.5)", new[]{-2.5f});
        
        [Test]
        public void AddWithIdentifiers() => AssertApproxEqual(CompilationInput, "Num.add(pi, 6)",  new[]{9.14f});
        
        [Test]
        public void NestedCallExpression() => AssertApproxEqual(CompilationInput, "Num.sub(Num.add(pi, 2), 5)", new[]{0.14f});
        
        [Test]
        public void CustomExpressionBodiedFunctionCall() => AssertApproxEqual(CompilationInput, "addThree(-3.14, -9, pi)", new[]{-9f});
    }
}