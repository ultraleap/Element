using NUnit.Framework;

namespace Laboratory.Tests.StandardLibrary
{
    internal class Vector4 : StandardLibraryFixture
    {
        [
            TestCase("Vector4.Zero", "Vector4(0, 0, 0, 0)"),
            TestCase("Vector4.One", "Vector4(1, 1, 1, 1)"),
        ]
        [
            TestCase("Vector4(0, 0, 0, 0).magnitudeSquared", "0"),
            TestCase("Vector4(1, 0, 0, 0).magnitudeSquared", "1"),
            TestCase("Vector4(2, 3, 6, 24).magnitudeSquared", "625"),
        ]
        [
            TestCase("Vector4(0, 0, 0, 0).magnitude", "0"),
            TestCase("Vector4(1, 0, 0, 0).magnitude", "1"),
            TestCase("Vector4(2, 3, 6, 24).magnitude", "25"),
        ]
        [
            TestCase("Vector4(0, 0, 0, 0).opposite", "Vector4(0, 0, 0, 0)"),
            TestCase("Vector4(1, 2, 3, 4).opposite", "Vector4(-1, -2, -3, -4)"),
            TestCase("Vector4(-1, 2, -3, 4).opposite", "Vector4(1, -2, 3, -4)"),
            TestCase("Vector4(1, -2, 3, -4).opposite", "Vector4(-1, 2, -3, 4)"),
            TestCase("Vector4(-1, -2, -3, -4).opposite", "Vector4(1, 2, 3, 4)"),
        ]
        [
            TestCase("Vector4(0, 0, 0, 0).normalize", "Vector4(Num.NaN, Num.NaN, Num.NaN, Num.NaN)"),
            TestCase("Vector4(1, 1, 1, 1).normalize", "Vector4(1.div(2), 1.div(2), 1.div(2), 1.div(2))"),
            TestCase("Vector4(2, 2, 4, 5).normalize", "Vector4(2.div(7), 2.div(7), 4.div(7), 5.div(7))"),
        ]
        [
            TestCase("Vector4(2, 4, 8, 16).mul(0)", "Vector4(0, 0, 0, 0)"),
            TestCase("Vector4(2, 4, 8, 16).mul(2)", "Vector4(4, 8, 16, 32)"),
            TestCase("Vector4(2, 4, 8, 16).mul(0.5)", "Vector4(1, 2, 4, 8)"),
            TestCase("Vector4(2, 4, 8, 16).mul(-2)", "Vector4(-4, -8, -16, -32)"),
            TestCase("Vector4(2, 4, 8, 16).mul(-0.5)", "Vector4(-1, -2, -4, -8)"),
            TestCase("Vector4(-2, -4, -8, -16).mul(2)", "Vector4(-4, -8, -16, -32)"),
            TestCase("Vector4(-2, -4, -8, -16).mul(0.5)", "Vector4(-1, -2, -4, -8)"),
            TestCase("Vector4(-2, -4, -8, -16).mul(-2)", "Vector4(4, 8, 16, 32)"),
            TestCase("Vector4(-2, -4, -8, -16).mul(-0.5)", "Vector4(1, 2, 4, 8)"),
        ]
        [
            TestCase("Vector4(2, 4, 8, 16).div(0)", "Vector4(Num.PositiveInfinity, Num.PositiveInfinity, Num.PositiveInfinity, Num.PositiveInfinity)"),
            TestCase("Vector4(2, 4, 8, 16).div(-0)", "Vector4(Num.NegativeInfinity, Num.NegativeInfinity, Num.NegativeInfinity, Num.NegativeInfinity)"),
            TestCase("Vector4(2, 4, 8, 16).div(2)", "Vector4(1, 2, 4, 8)"),
            TestCase("Vector4(2, 4, 8, 16).div(0.5)", "Vector4(4, 8, 16, 32)"),
            TestCase("Vector4(2, 4, 8, 16).div(-2)", "Vector4(-1, -2, -4, -8)"),
            TestCase("Vector4(2, 4, 8, 16).div(-0.5)", "Vector4(-4, -8, -16, -32)"),
            TestCase("Vector4(-2, -4, -8, -16).div(2)", "Vector4(-1, -2, -4, -8)"),
            TestCase("Vector4(-2, -4, -8, -16).div(0.5)", "Vector4(-4, -8, -16, -32)"),
            TestCase("Vector4(-2, -4, -8, -16).div(-2)", "Vector4(1, 2, 4, 8)"),
            TestCase("Vector4(-2, -4, -8, -16).div(-0.5)", "Vector4(4, 8, 16, 32)"),
        ]
        [
            TestCase("Vector4(0, 0, 0, 0).add(Vector4(0, 0, 0, 0))", "Vector4(0, 0, 0, 0)"),
            TestCase("Vector4(1, 2, 3, 4).add(Vector4(0, 0, 0, 0))", "Vector4(1, 2, 3, 4)"),
            TestCase("Vector4(1, 2, 3, 4).add(Vector4(2, 1, 3, 6))", "Vector4(3, 3, 6, 10)"),
            TestCase("Vector4(1, 2, 3, 4).add(Vector4(-2, -1, -3, -6))", "Vector4(-1, 1, 0, -2)"),
        ]
        [
            TestCase("Vector4(0, 0, 0, 0).sub(Vector4(0, 0, 0, 0))", "Vector4(0, 0, 0, 0)"),
            TestCase("Vector4(1, 2, 3, 4).sub(Vector4(0, 0, 0, 0))", "Vector4(1, 2, 3, 4)"),
            TestCase("Vector4(1, 2, 3, 4).sub(Vector4(2, 1, 3, 6))", "Vector4(-1, 1, 0, -2)"),
            TestCase("Vector4(1, 2, 3, 4).sub(Vector4(-2, -1, -3, -6))", "Vector4(3, 3, 6, 10)"),
        ]
        [
            TestCase("Vector4(0, 0, 0, 0).dot(Vector4(0, 0, 0, 0))", "0"),
            TestCase("Vector4(1, 1, 1, 9).dot(Vector4(0, 0, 0, 0))", "0"),
            TestCase("Vector4(1, 3, 7, 9).dot(Vector4(2, 5, 4, 5))", "90"),
            TestCase("Vector4(1, 3, 7, 9).dot(Vector4(-2, -5, -4, -5))", "-90"),
        ]
        public void Operations(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
    }
}