using NUnit.Framework;

namespace Laboratory.Tests.StandardLibrary
{
    internal class Vector3 : StandardLibraryFixture
    {
        [
            TestCase("Vector3.Zero", "Vector3(0, 0, 0)"),
            TestCase("Vector3.One", "Vector3(1, 1, 1)"),
            TestCase("Vector3.Up", "Vector3(0, 0, 1)"),
            TestCase("Vector3.Down", "Vector3(0, 0, -1)"),
            TestCase("Vector3.Right", "Vector3(1, 0, 0)"),
            TestCase("Vector3.Left", "Vector3(-1, 0, 0)"),
            TestCase("Vector3.Forward", "Vector3(0, 1, 0)"),
            TestCase("Vector3.Back", "Vector3(0, -1, 0)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).magnitudeSquared", "0"),
            TestCase("Vector3(1, 0, 0).magnitudeSquared", "1"),
            TestCase("Vector3(2, 3, 6).magnitudeSquared", "49"),
        ]
        [
            TestCase("Vector3(0, 0, 0).magnitude", "0"),
            TestCase("Vector3(1, 0, 0).magnitude", "1"),
            TestCase("Vector3(1, 2, 2).magnitude", "3"),
        ]
        [
            TestCase("Vector3(0, 0, 0).opposite", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 2, 3).opposite", "Vector3(-1, -2, -3)"),
            TestCase("Vector3(-1, 2, -3).opposite", "Vector3(1, -2, 3)"),
            TestCase("Vector3(1, -2, 3).opposite", "Vector3(-1, 2, -3)"),
            TestCase("Vector3(-1, -2, -3).opposite", "Vector3(1, 2, 3)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).normalize", "Vector3(Num.NaN, Num.NaN, Num.NaN)"),
            TestCase("Vector3(1, 1, 1).normalize", "Vector3(1.div(3.sqrt), 1.div(3.sqrt), 1.div(3.sqrt))"),
            TestCase("Vector3(1, 2, 2).normalize", "Vector3(1.div(3), 2.div(3), 2.div(3))"),
        ]
        [
            TestCase("Vector3(2, 4, 8).mul(0)", "Vector3(0, 0, 0)"),
            TestCase("Vector3(2, 4, 8).mul(2)", "Vector3(4, 8, 16)"),
            TestCase("Vector3(2, 4, 8).mul(0.5)", "Vector3(1, 2, 4)"),
            TestCase("Vector3(2, 4, 8).mul(-2)", "Vector3(-4, -8, -16)"),
            TestCase("Vector3(2, 4, 8).mul(-0.5)", "Vector3(-1, -2, -4)"),
            TestCase("Vector3(-2, -4, -8).mul(2)", "Vector3(-4, -8, -16)"),
            TestCase("Vector3(-2, -4, -8).mul(0.5)", "Vector3(-1, -2, -4)"),
            TestCase("Vector3(-2, -4, -8).mul(-2)", "Vector3(4, 8, 16)"),
            TestCase("Vector3(-2, -4, -8).mul(-0.5)", "Vector3(1, 2, 4)"),
        ]
        [
            TestCase("Vector3(2, 4, 8).div(0)", "Vector3(Num.PositiveInfinity, Num.PositiveInfinity, Num.PositiveInfinity)"),
            TestCase("Vector3(2, 4, 8).div(-0)", "Vector3(Num.NegativeInfinity, Num.NegativeInfinity, Num.NegativeInfinity)"),
            TestCase("Vector3(2, 4, 8).div(2)", "Vector3(1, 2, 4)"),
            TestCase("Vector3(2, 4, 8).div(0.5)", "Vector3(4, 8, 16)"),
            TestCase("Vector3(2, 4, 8).div(-2)", "Vector3(-1, -2, -4)"),
            TestCase("Vector3(2, 4, 8).div(-0.5)", "Vector3(-4, -8, -16)"),
            TestCase("Vector3(-2, -4, -8).div(2)", "Vector3(-1, -2, -4)"),
            TestCase("Vector3(-2, -4, -8).div(0.5)", "Vector3(-4, -8, -16)"),
            TestCase("Vector3(-2, -4, -8).div(-2)", "Vector3(1, 2, 4)"),
            TestCase("Vector3(-2, -4, -8).div(-0.5)", "Vector3(4, 8, 16)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).add(Vector3(0, 0, 0))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 2, 3).add(Vector3(0, 0, 0))", "Vector3(1, 2, 3)"),
            TestCase("Vector3(1, 2, 3).add(Vector3(2, 1, 3))", "Vector3(3, 3, 6)"),
            TestCase("Vector3(1, 2, 3).add(Vector3(-2, -1, -3))", "Vector3(-1, 1, 0)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).sub(Vector3(0, 0, 0))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 2, 3).sub(Vector3(0, 0, 0))", "Vector3(1, 2, 3)"),
            TestCase("Vector3(1, 2, 3).sub(Vector3(2, 1, 3))", "Vector3(-1, 1, 0)"),
            TestCase("Vector3(1, 2, 3).sub(Vector3(-2, -1, -3))", "Vector3(3, 3, 6)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).dot(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(1, 1, 1).dot(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(1, 3, 7).dot(Vector3(2, 5, 4))", "45"),
            TestCase("Vector3(1, 3, 7).dot(Vector3(-2, -5, -4))", "-45"),
        ]
        [
            TestCase("Vector3(0, 0, 0).distance(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(0, 0, 0).distance(Vector3(2, 3, 6))", "7"),
        ]
        [
            TestCase("Vector3(0, 0, 0).angle(Vector3(0, 0, 0))", "Num.NaN"),
            TestCase("Vector3(0, 1, 1).angle(Vector3(0, 1, 1))", "0"),
            TestCase("Vector3(0, 1, 1).angle(Vector3(1, 1, 0))", "60"),
            TestCase("Vector3(0, 1, 1).angle(Vector3(-1, 0, 1))", "60"),
            TestCase("Vector3(0, 1, 1).angle(Vector3(1, -1, 1))", "90"),
            TestCase("Vector3(0, 1, 1).angle(Vector3(-1, -1, 0))", "120"),
        ]
        [
            TestCase("Vector3(1, 1, 1).reflect(Vector3(0, 1, 0))", "Vector3(-1, 1, -1)"),
            TestCase("Vector3(-1, 1, 1).reflect(Vector3(0, 1, 0))", "Vector3(1, 1, -1)"),
            TestCase("Vector3(1, -1, 1).reflect(Vector3(0, 1, 0))", "Vector3(-1, -1, -1)"),
            TestCase("Vector3(-1, -1, 1).reflect(Vector3(0, 1, 0))", "Vector3(1, -1, -1)"),
            TestCase("Vector3(1, 1, -1).reflect(Vector3(1, 0, 0))", "Vector3(1, -1, 1)"),
            TestCase("Vector3(-1, 1, -1).reflect(Vector3(1, 0, 0))", "Vector3(-1, -1, 1)"),
            TestCase("Vector3(1, -1, -1).reflect(Vector3(1, 0, 0))", "Vector3(1, 1, 1)"),
            TestCase("Vector3(-1, -1, -1).reflect(Vector3(1, 0, 0))", "Vector3(-1, 1, 1)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).cross(Vector3(0, 0, 0))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 1, 1).cross(Vector3(1, 1, 1))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 1, 1).cross(Vector3(1, 1, 0))", "Vector3(-1, 1, 0)"),
            TestCase("Vector3(3, 4, 5).cross(Vector3(1, 0, 0))", "Vector3(0, 5, -4)"),
        ]
        public void Operations(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
    }
}