using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class NestedConstructs : SemanticsFixture
    {
        public NestedConstructs(IHost host) : base(host, "NestedConstructs") { }

        [Test]
        public void AddUsingLocal() => AssertApproxEqual(CompilationInput, "addUsingLocal(6, 6)", "12");

        [Test]
        public void AddUsingCaptured() => AssertApproxEqual(CompilationInput, "addUsingLocalCapture(6, 6)", "12");

        [Test]
        public void LocalStructInstance() => AssertTypeof(CompilationInput, "returnLocalStructInstance(5)", "Vector2");

        [Test]
        public void IndirectlyRecursive() =>;
    }
}