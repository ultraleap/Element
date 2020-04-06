using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class Vector3 : StandardLibraryFixture
    {
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
        // [
        //     TestCase("Vector2(2, 4).Mul(0)", "Vector2(0, 0)"),
        //     TestCase("Vector2(2, 4).Mul(2)", "Vector2(4, 8)"),
        //     TestCase("Vector2(2, 4).Mul(0.5)", "Vector2(1, 2)"),
        //     TestCase("Vector2(2, 4).Mul(-2)", "Vector2(-4, -8)"),
        //     TestCase("Vector2(2, 4).Mul(-0.5)", "Vector2(-1, -2)"),
        //     TestCase("Vector2(-2, 4).Mul(2)", "Vector2(-4, 8)"),
        //     TestCase("Vector2(-2, 4).Mul(0.5)", "Vector2(-1, 2)"),
        //     TestCase("Vector2(-2, 4).Mul(-2)", "Vector2(4, -8)"),
        //     TestCase("Vector2(-2, 4).Mul(-0.5)", "Vector2(1, -2)"),
        //     TestCase("Vector2(2, -4).Mul(2)", "Vector2(4, -8)"),
        //     TestCase("Vector2(2, -4).Mul(0.5)", "Vector2(1, -2)"),
        //     TestCase("Vector2(2, -4).Mul(-2)", "Vector2(-4, 8)"),
        //     TestCase("Vector2(2, -4).Mul(-0.5)", "Vector2(-1, 2)"),
        //     TestCase("Vector2(-2, -4).Mul(2)", "Vector2(-4, -8)"),
        //     TestCase("Vector2(-2, -4).Mul(0.5)", "Vector2(-1, -2)"),
        //     TestCase("Vector2(-2, -4).Mul(-2)", "Vector2(4, 8)"),
        //     TestCase("Vector2(-2, -4).Mul(-0.5)", "Vector2(1, 2)"),
        // ]
        // [
        //     TestCase("Vector2(2, 4).Div(0)", "Vector2(Num.PositiveInfinity, Num.PositiveInfinity)"),
        //     TestCase("Vector2(2, 4).Div(-0)", "Vector2(Num.NegativeInfinity, Num.NegativeInfinity)"),
        //     TestCase("Vector2(2, 4).Div(2)", "Vector2(1, 2)"),
        //     TestCase("Vector2(2, 4).Div(0.5)", "Vector2(4, 8)"),
        //     TestCase("Vector2(2, 4).Div(-2)", "Vector2(-1, -2)"),
        //     TestCase("Vector2(2, 4).Div(-0.5)", "Vector2(-4, -8)"),
        //     TestCase("Vector2(-2, 4).Div(2)", "Vector2(-1, 2)"),
        //     TestCase("Vector2(-2, 4).Div(0.5)", "Vector2(-4, 8)"),
        //     TestCase("Vector2(-2, 4).Div(-2)", "Vector2(1, -2)"),
        //     TestCase("Vector2(-2, 4).Div(-0.5)", "Vector2(4, -8)"),
        //     TestCase("Vector2(2, -4).Div(2)", "Vector2(1, -2)"),
        //     TestCase("Vector2(2, -4).Div(0.5)", "Vector2(4, -8)"),
        //     TestCase("Vector2(2, -4).Div(-2)", "Vector2(-1, 2)"),
        //     TestCase("Vector2(2, -4).Div(-0.5)", "Vector2(-4, 8)"),
        //     TestCase("Vector2(-2, -4).Div(2)", "Vector2(-1, -2)"),
        //     TestCase("Vector2(-2, -4).Div(0.5)", "Vector2(-4, -8)"),
        //     TestCase("Vector2(-2, -4).Div(-2)", "Vector2(1, 2)"),
        //     TestCase("Vector2(-2, -4).Div(-0.5)", "Vector2(4, 8)"),
        // ]
        // [
        //     TestCase("Vector2(0, 0).Add(Vector2(0, 0))", "Vector2(0, 0)"),
        //     TestCase("Vector2(1, 2).Add(Vector2(2, 1))", "Vector2(3, 3)"),
        //     TestCase("Vector2(1, 2).Add(Vector2(-2, -1))", "Vector2(-1, 1)"),
        // ]
        // [
        //     TestCase("Vector2(0, 0).Sub(Vector2(0, 0))", "Vector2(0, 0)"),
        //     TestCase("Vector2(1, 2).Sub(Vector2(2, 1))", "Vector2(-1, 1)"),
        //     TestCase("Vector2(1, 2).Sub(Vector2(-2, -1))", "Vector2(3, 3)"),
        // ]
        [
            TestCase("Vector3(0, 0, 0).Dot(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(1, 1, 1).Dot(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(1, 3, 7).Dot(Vector3(2, 5, 4))", "45"),
            TestCase("Vector3(1, 3, 7).Dot(Vector3(-2, -5, -4))", "-45"),
        ]
        // [
        //     TestCase("Vector2(0, 0).Distance(Vector2(0, 0))", "0"),
        //     TestCase("Vector2(0, 0).Distance(Vector2(3, 4))", "5"),
        // ]
        // [
        //     TestCase("Vector2(0, 0).Angle(Vector2(0, 0))", "Num.NaN"),
        //     TestCase("Vector2(0, 1).Angle(Vector2(0, 1))", "0"),
        //     TestCase("Vector2(0, 1).Angle(Vector2(1, 1))", "45"),
        //     TestCase("Vector2(0, 1).Angle(Vector2(-1, 1))", "45"),
        //     TestCase("Vector2(0, 1).Angle(Vector2(1, -1))", "135"),
        //     TestCase("Vector2(0, 1).Angle(Vector2(-1, -1))", "135"),
        // ]
        // [
        //     TestCase("Vector2(1, 1).Reflect(Vector2(0, 1))", "Vector2(-1, 1)"),
        //     TestCase("Vector2(-1, 1).Reflect(Vector2(0, 1))", "Vector2(1, 1)"),
        //     TestCase("Vector2(1, -1).Reflect(Vector2(0, 1))", "Vector2(-1, -1)"),
        //     TestCase("Vector2(-1, -1).Reflect(Vector2(0, 1))", "Vector2(1, -1)"),
        //     TestCase("Vector2(1, 1).Reflect(Vector2(1, 0))", "Vector2(1, -1)"),
        //     TestCase("Vector2(-1, 1).Reflect(Vector2(1, 0))", "Vector2(-1, -1)"),
        //     TestCase("Vector2(1, -1).Reflect(Vector2(1, 0))", "Vector2(1, 1)"),
        //     TestCase("Vector2(-1, -1).Reflect(Vector2(1, 0))", "Vector2(-1, 1)"),
        // ]
        public void Operations(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expression, expected);
    }
}