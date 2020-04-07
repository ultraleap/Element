using NUnit.Framework;

namespace Laboratory.Tests.Semantics
{
    internal class IdentifierResolution : SemanticsFixture
    {
        public IdentifierResolution() :base("IdentifierResolution") { }

        [
            TestCase("A.A.a", "15"),
            TestCase("A.A.b", "20"),
            TestCase("X.Y.a", "5"),
            TestCase("X.Y.b", "10"),
            TestCase("X.Y.c", "15"),
        ]
        public void ResolvesCorrectLiteral(string expression, string expected) => AssertApproxEqual(CompilationInput, expression, expected);

        [Test]
        public void ResolveResultingNullaryExpression() => AssertApproxEqual(CompilationInput, "A.x", "5");
    }
}