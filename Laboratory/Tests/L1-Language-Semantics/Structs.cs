using System.IO;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class Structs : HostFixture
    {
        public Structs(IHost host) : base(host) { }
        
        private static FileInfo[] SourceFiles => new []{GetTestFile("Structs")};

        private static CompilationInput CompilationInput => new CompilationInput(FailOnError, true, extraSourceFiles: SourceFiles);

        [Test]
        public void ConstructInstance() => _host.Evaluate(CompilationInput, "MyStruct(5)");
        
        [Test]
        public void ConstructInstanceWithNestedStruct() => _host.Evaluate(CompilationInput, "NestedStruct(MyStruct(5))");

        [Test]
        public void InstanceMemberAccess() => AssertApproxEqual(CompilationInput, "MyStruct(5).a", "5");

        [Test]
        public void InvalidMemberAccess() => EvaluateExpectingErrorCode(CompilationInput, 7, "MyStruct(10).invalid");

        [Test]
        public void FunctionAsMember() => _host.Evaluate(CompilationInput, "MyStruct(pickSecond)");

        [Test]
        public void ConstrainedMembers() => _host.Evaluate(CompilationInput, "Vector3(5, 10, 15)");

        [Test]
        public void MemberConstraintsNotSatisfied() => EvaluateExpectingErrorCode(CompilationInput, 8, "Vector3(pickSecond, 5, 10)");
    }
}