using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Line : StandardLibraryFixture
    {
        [
            TestCase("Line2(Vector2(1, -1), Vector2(5, 5)).distanceFromPoint(Vector2(-5, 5))", "10.div(2.sqrt)"),
        ]
        [
            TestCase("Line3(Vector3(1, -1, 0), Vector3(5, 5, 0)).distanceFromPoint(Vector3(-5, 5, 0))", "10.div(2.sqrt)"),
        ]
        public void Operations(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
    }
}