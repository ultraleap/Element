using NUnit.Framework;

namespace Laboratory.Tests.StandardLibrary
{
    internal class Vector2 : StandardLibraryFixture
    {
        [
            TestCase("Vector2.Zero", "Vector2(0, 0)"),
            TestCase("Vector2.One", "Vector2(1, 1)"),
            //TestCase("Vector2.Right", "Vector3(1, 0)"),
            //TestCase("Vector2.Left", "Vector3(-1, 0)"),
            //TestCase("Vector2.Up", "Vector3(0, 1)"),
            //TestCase("Vector2.Down", "Vector3(0, -1)"),
        ]
        [
            TestCase("Vector2(0, 0).magnitudeSquared", "0"),
            TestCase("Vector2(1, 0).magnitudeSquared", "1"),
            TestCase("Vector2(3, 4).magnitudeSquared", "25"),
        ]
        [
            TestCase("Vector2(0, 0).magnitude", "0"),
            TestCase("Vector2(1, 0).magnitude", "1"),
            TestCase("Vector2(3, 4).magnitude", "5"),
        ]
        [
            TestCase("Vector2(0, 0).opposite", "Vector2(0, 0)"),
            TestCase("Vector2(1, 2).opposite", "Vector2(-1, -2)"),
            TestCase("Vector2(-1, 2).opposite", "Vector2(1, -2)"),
            TestCase("Vector2(1, -2).opposite", "Vector2(-1, 2)"),
            TestCase("Vector2(-1, -2).opposite", "Vector2(1, 2)"),
        ]
        [
            TestCase("Vector2(0, 0).normalize", "Vector2(Num.NaN, Num.NaN)"),
            TestCase("Vector2(1, 1).normalize", "Vector2(0.707106769, 0.707106769)"),
            TestCase("Vector2(3, 4).normalize", "Vector2(0.6, 0.8)"),
        ]
        [
            TestCase("Vector2(2, 4).mul(0)", "Vector2(0, 0)"),
            TestCase("Vector2(2, 4).mul(2)", "Vector2(4, 8)"),
            TestCase("Vector2(2, 4).mul(0.5)", "Vector2(1, 2)"),
            TestCase("Vector2(2, 4).mul(-2)", "Vector2(-4, -8)"),
            TestCase("Vector2(2, 4).mul(-0.5)", "Vector2(-1, -2)"),
            TestCase("Vector2(-2, -4).mul(2)", "Vector2(-4, -8)"),
            TestCase("Vector2(-2, -4).mul(0.5)", "Vector2(-1, -2)"),
            TestCase("Vector2(-2, -4).mul(-2)", "Vector2(4, 8)"),
            TestCase("Vector2(-2, -4).mul(-0.5)", "Vector2(1, 2)"),
        ]
        [
            TestCase("Vector2(2, 4).div(0)", "Vector2(Num.PositiveInfinity, Num.PositiveInfinity)"),
            TestCase("Vector2(2, 4).div(-0)", "Vector2(Num.NegativeInfinity, Num.NegativeInfinity)"),
            TestCase("Vector2(2, 4).div(2)", "Vector2(1, 2)"),
            TestCase("Vector2(2, 4).div(0.5)", "Vector2(4, 8)"),
            TestCase("Vector2(2, 4).div(-2)", "Vector2(-1, -2)"),
            TestCase("Vector2(2, 4).div(-0.5)", "Vector2(-4, -8)"),
            TestCase("Vector2(-2, -4).div(2)", "Vector2(-1, -2)"),
            TestCase("Vector2(-2, -4).div(0.5)", "Vector2(-4, -8)"),
            TestCase("Vector2(-2, -4).div(-2)", "Vector2(1, 2)"),
            TestCase("Vector2(-2, -4).div(-0.5)", "Vector2(4, 8)"),
        ]
        [
            TestCase("Vector2(0, 0).add(Vector2(0, 0))", "Vector2(0, 0)"),
            TestCase("Vector2(1, 2).add(Vector2(0, 0))", "Vector2(1, 2)"),
            TestCase("Vector2(1, 2).add(Vector2(2, 1))", "Vector2(3, 3)"),
            TestCase("Vector2(1, 2).add(Vector2(-2, -1))", "Vector2(-1, 1)"),
        ]
        [
            TestCase("Vector2(0, 0).sub(Vector2(0, 0))", "Vector2(0, 0)"),
            TestCase("Vector2(1, 2).sub(Vector2(0, 0))", "Vector2(1, 2)"),
            TestCase("Vector2(1, 2).sub(Vector2(2, 1))", "Vector2(-1, 1)"),
            TestCase("Vector2(1, 2).sub(Vector2(-2, -1))", "Vector2(3, 3)"),
        ]
        [
            TestCase("Vector2(0, 0).dot(Vector2(0, 0))", "0"),
            TestCase("Vector2(1, 1).dot(Vector2(0, 0))", "0"),
            TestCase("Vector2(1, 3).dot(Vector2(2, 5))", "17"),
            TestCase("Vector2(1, 3).dot(Vector2(-2, -5))", "-17"),
        ]
        [
            TestCase("Vector2(0, 0).distance(Vector2(0, 0))", "0"),
            TestCase("Vector2(0, 0).distance(Vector2(3, 4))", "5"),
        ]
        [
            TestCase("Vector2(0, 0).angle(Vector2(0, 0))", "Num.NaN"),
            TestCase("Vector2(0, 1).angle(Vector2(0, 1))", "0"),
            TestCase("Vector2(0, 1).angle(Vector2(1, 1))", "45"),
            TestCase("Vector2(0, 1).angle(Vector2(-1, 1))", "45"),
            TestCase("Vector2(0, 1).angle(Vector2(1, -1))", "135"),
            TestCase("Vector2(0, 1).angle(Vector2(-1, -1))", "135"),
        ]
        [
            TestCase("Vector2(1, 1).reflect(Vector2(0, 1))", "Vector2(-1, 1)"),
            TestCase("Vector2(-1, 1).reflect(Vector2(0, 1))", "Vector2(1, 1)"),
            TestCase("Vector2(1, -1).reflect(Vector2(0, 1))", "Vector2(-1, -1)"),
            TestCase("Vector2(-1, -1).reflect(Vector2(0, 1))", "Vector2(1, -1)"),
            TestCase("Vector2(1, 1).reflect(Vector2(1, 0))", "Vector2(1, -1)"),
            TestCase("Vector2(-1, 1).reflect(Vector2(1, 0))", "Vector2(-1, -1)"),
            TestCase("Vector2(1, -1).reflect(Vector2(1, 0))", "Vector2(1, 1)"),
            TestCase("Vector2(-1, -1).reflect(Vector2(1, 0))", "Vector2(-1, 1)"),
        ]
        public void Operations(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
    }
}