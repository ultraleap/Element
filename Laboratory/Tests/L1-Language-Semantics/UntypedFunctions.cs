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
        public void Sub() => AssertApproxEqual(CompilationInput, "b", -2.5f);
        
        [Test]
        public void AddWithIdentifiers() => AssertApproxEqual(CompilationInput, "c", 10.5f);
        
        [Test]
        public void NestedCallExpression() => AssertApproxEqual(CompilationInput, "d", 6f);
        
        [Test]
        public void CustomExpressionBodiedFunctionCall() => AssertApproxEqual(CompilationInput, "e", 18f);
    
        [Test]
        public void CustomScopeBodiedFunctionCall() => AssertApproxEqual(CompilationInput, "f", 23f);
    }
}