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
        public void Add() => AssertApproxEqual(CompilationInput, "a", 13f);
        
        [Test]
        public void Sub() => AssertApproxEqual(CompilationInput, "b", -1f);
        
        [Test]
        public void AddWithIdentifiers() => AssertApproxEqual(CompilationInput, "c", 12f);
        
        [Test]
        public void NestedCallExpression() => AssertApproxEqual(CompilationInput, "d", 9f);
        
        [Test]
        public void CustomExpressionBodiedFunctionCall() => AssertApproxEqual(CompilationInput, "e", 22.5f);
    
        [Test]
        public void CustomScopeBodiedFunctionCall() => AssertApproxEqual(CompilationInput, "e", 22.5f);
    }
}