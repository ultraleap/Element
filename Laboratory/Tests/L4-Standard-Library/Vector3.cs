using NUnit.Framework;

namespace Laboratory.Tests
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
            TestCase("Vector3(0, 0, 0).MagnitudeSquared", "0"),
            TestCase("Vector3(1, 0, 0).MagnitudeSquared", "1"),
            TestCase("Vector3(2, 3, 6).MagnitudeSquared", "49"),
        ]
        [
            TestCase("Vector3(0, 0, 0).Magnitude", "0"),
            TestCase("Vector3(1, 0, 0).Magnitude", "1"),
            TestCase("Vector3(1, 2, 2).Magnitude", "3"),
        ]
        [
            TestCase("Vector3(0, 0, 0).Opposite", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 2, 3).Opposite", "Vector3(-1, -2, -3)"),
            TestCase("Vector3(-1, 2, -3).Opposite", "Vector3(1, -2, 3)"),
            TestCase("Vector3(1, -2, 3).Opposite", "Vector3(-1, 2, -3)"),
            TestCase("Vector3(-1, -2, -3).Opposite", "Vector3(1, 2, 3)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).Normalize", "Vector3(Num.NaN, Num.NaN, Num.NaN)"),
            TestCase("Vector3(1, 1, 1).Normalize", "Vector3(0.57735026919, 0.57735026919, 0.57735026919)"),
            TestCase("Vector3(1, 2, 2).Normalize", "Vector3(0.33333333333, 0.66666666666, 0.66666666666)"),
        ]
        [
            TestCase("Vector3(2, 4, 8).Mul(0)", "Vector3(0, 0, 0)"),
            TestCase("Vector3(2, 4, 8).Mul(2)", "Vector3(4, 8, 16)"),
            TestCase("Vector3(2, 4, 8).Mul(0.5)", "Vector3(1, 2, 4)"),
            TestCase("Vector3(2, 4, 8).Mul(-2)", "Vector3(-4, -8, -16)"),
            TestCase("Vector3(2, 4, 8).Mul(-0.5)", "Vector3(-1, -2, -4)"),
            TestCase("Vector3(-2, -4, -8).Mul(2)", "Vector3(-4, -8, -16)"),
            TestCase("Vector3(-2, -4, -8).Mul(0.5)", "Vector3(-1, -2, -4)"),
            TestCase("Vector3(-2, -4, -8).Mul(-2)", "Vector3(4, 8, 16)"),
            TestCase("Vector3(-2, -4, -8).Mul(-0.5)", "Vector3(1, 2, 4)"),
        ]
        [
            TestCase("Vector3(2, 4, 8).Div(0)", "Vector3(Num.PositiveInfinity, Num.PositiveInfinity, Num.PositiveInfinity)"),
            TestCase("Vector3(2, 4, 8).Div(-0)", "Vector3(Num.NegativeInfinity, Num.NegativeInfinity, Num.NegativeInfinity)"),
            TestCase("Vector3(2, 4, 8).Div(2)", "Vector3(1, 2, 4)"),
            TestCase("Vector3(2, 4, 8).Div(0.5)", "Vector3(4, 8, 16)"),
            TestCase("Vector3(2, 4, 8).Div(-2)", "Vector3(-1, -2, -4)"),
            TestCase("Vector3(2, 4, 8).Div(-0.5)", "Vector3(-4, -8, -16)"),
            TestCase("Vector3(-2, -4, -8).Div(2)", "Vector3(-1, -2, -4)"),
            TestCase("Vector3(-2, -4, -8).Div(0.5)", "Vector3(-4, -8, -16)"),
            TestCase("Vector3(-2, -4, -8).Div(-2)", "Vector3(1, 2, 4)"),
            TestCase("Vector3(-2, -4, -8).Div(-0.5)", "Vector3(4, 8, 16)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).Add(Vector3(0, 0, 0))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 2, 3).Add(Vector3(0, 0, 0))", "Vector3(1, 2, 3)"),
            TestCase("Vector3(1, 2, 3).Add(Vector3(2, 1, 3))", "Vector3(3, 3, 6)"),
            TestCase("Vector3(1, 2, 3).Add(Vector3(-2, -1, -3))", "Vector3(-1, 1, 0)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).Sub(Vector3(0, 0, 0))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 2, 3).Sub(Vector3(0, 0, 0))", "Vector3(1, 2, 3)"),
            TestCase("Vector3(1, 2, 3).Sub(Vector3(2, 1, 3))", "Vector3(-1, 1, 0)"),
            TestCase("Vector3(1, 2, 3).Sub(Vector3(-2, -1, -3))", "Vector3(3, 3, 6)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).Dot(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(1, 1, 1).Dot(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(1, 3, 7).Dot(Vector3(2, 5, 4))", "45"),
            TestCase("Vector3(1, 3, 7).Dot(Vector3(-2, -5, -4))", "-45"),
        ]
        [
            TestCase("Vector3(0, 0, 0).Distance(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(0, 0, 0).Distance(Vector3(2, 3, 6))", "7"),
        ]
        [
            TestCase("Vector3(0, 0, 0).Angle(Vector3(0, 0, 0))", "Num.NaN"),
            TestCase("Vector3(0, 1, 1).Angle(Vector3(0, 1, 1))", "0"),
            TestCase("Vector3(0, 1, 1).Angle(Vector3(1, 1, 0))", "60"),
            TestCase("Vector3(0, 1, 1).Angle(Vector3(-1, 0, 1))", "60"),
            TestCase("Vector3(0, 1, 1).Angle(Vector3(1, -1, 1))", "90"),
            TestCase("Vector3(0, 1, 1).Angle(Vector3(-1, -1, 0))", "120"),
        ]
        [
            TestCase("Vector3(1, 1, 1).Reflect(Vector3(0, 1, 0))", "Vector3(-1, 1, -1)"),
            TestCase("Vector3(-1, 1, 1).Reflect(Vector3(0, 1, 0))", "Vector3(1, 1, -1)"),
            TestCase("Vector3(1, -1, 1).Reflect(Vector3(0, 1, 0))", "Vector3(-1, -1, -1)"),
            TestCase("Vector3(-1, -1, 1).Reflect(Vector3(0, 1, 0))", "Vector3(1, -1, -1)"),
            TestCase("Vector3(1, 1, -1).Reflect(Vector3(1, 0, 0))", "Vector3(1, -1, 1)"),
            TestCase("Vector3(-1, 1, -1).Reflect(Vector3(1, 0, 0))", "Vector3(-1, -1, 1)"),
            TestCase("Vector3(1, -1, -1).Reflect(Vector3(1, 0, 0))", "Vector3(1, 1, 1)"),
            TestCase("Vector3(-1, -1, -1).Reflect(Vector3(1, 0, 0))", "Vector3(-1, 1, 1)"),
        ]
        [
            TestCase("Vector3(0, 0, 0).Cross(Vector3(0, 0, 0))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 1, 1).Cross(Vector3(1, 1, 1))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 1, 1).Cross(Vector3(1, 1, 0))", "Vector3(-1, 1, 0)"),
            TestCase("Vector3(3, 4, 5).Cross(Vector3(1, 0, 0))", "Vector3(0, 5, -4)"),
        ]
        public void Operations(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
    }
}