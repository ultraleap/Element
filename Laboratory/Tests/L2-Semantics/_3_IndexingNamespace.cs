using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class _3_IndexingNamespace : SemanticsFixture
    {
        public _3_IndexingNamespace() :base("_3_IndexingNamespace") { }

        [
            TestCase("A.x", "5"),
            TestCase("A.A.x", "10"),
            TestCase("A.A.A.x", "15"),
            TestCase("A.A.A.A.x", "20"),
            TestCase("A.A.a", "15"),
            TestCase("A.A.b", "20"),
            TestCase("X.x", "5"),
            TestCase("X.Y.x", "10"),
            TestCase("X.Y.Z.x", "15"),
            TestCase("X.Y.a", "5"),
            TestCase("X.Y.b", "10"),
            TestCase("X.Y.c", "15"),
        ]
        public void IndexingNamespace(string expression, string expected) => AssertApproxEqual(CompilationInput, expression, expected);

        [TestCase("A.foo")]
        [TestCase("A.A.c")]
        public void IndexingIdentifierNotFound(string expression) => EvaluateExpectingErrorCode(CompilationInput, MessageCode.IdentifierNotFound, expression);
        
        [TestCase("A", "Namespace")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilationInput, expression, type);
    }
}