using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
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
        public void ResolvesCorrectLiteralAfterIndexingNamespace(string expression, string expected) => AssertApproxEqual(CompilationInput, expression, expected);
        
        [
            TestCase("Num.acos(0).degrees", "90"),
        ]
        public void ResolvesCorrectLiteralAfterIndexingIntrinsicFunction(string expression, string expected) => AssertApproxEqual(CompilationInput, expression, expected);

        [Test]
        public void ResolveResultingNullaryExpression() => AssertApproxEqual(CompilationInput, "A.x", "5");
    }
}