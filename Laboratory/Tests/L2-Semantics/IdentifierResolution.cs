using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class IdentifierResolution : SemanticsFixture
    {
        public IdentifierResolution(IHost host) :base(host, "IdentifierResolution") { }

        [
            TestCase("A.A.a", 15f),
            TestCase("A.A.b", 20f),
            TestCase("X.Y.a", 5f),
            TestCase("X.Y.b", 10f),
            TestCase("X.Y.c", 15f),
        ]
        public void ResolvesCorrectLiteral(string expression, float literal) => AssertApproxEqual(CompilationInput, expression, new[]{literal});

        [Test]
        public void ResolveResultingNullaryExpression() => AssertApproxEqual(CompilationInput, "A.x", new []{5f});
    }
}