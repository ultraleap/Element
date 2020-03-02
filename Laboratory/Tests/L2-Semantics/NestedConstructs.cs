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
        public void AddUsingLocalWithCaptures() => AssertApproxEqual(CompilationInput, "addUsingLocalCapture(6, 6)", "12");

        [Test]
        public void AddUsingLocalWithShadowing() => AssertApproxEqual(CompilationInput, "addUsingLocalWithShadowing(6, 6)", "12");

        [Test]
        public void LocalStructInstance() => AssertTypeof(CompilationInput, "returnLocalStructInstance(5)", "Vector2");

        [Test]
        public void LocalStructInstanceFunction() => AssertApproxEqual(CompilationInput, "returnLocalStructInstance(5).add(10).x", "15");
    }
}