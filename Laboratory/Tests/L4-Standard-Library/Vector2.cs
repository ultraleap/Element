using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class Vector2 : StandardLibraryFixture
    {
        [
            TestCase("Vector2(0, 0).MagnitudeSquared", "0"),
            TestCase("Vector2(1, 0).MagnitudeSquared", "1"),
            TestCase("Vector2(3, 4).MagnitudeSquared", "25"),
        ]
        [
            TestCase("Vector2(0, 0).Magnitude", "0"),
            TestCase("Vector2(1, 0).Magnitude", "1"),
            TestCase("Vector2(3, 4).Magnitude", "5"),
        ]
        [
            TestCase("Vector2(0, 0).Opposite", "Vector2(0, 0)"),
            TestCase("Vector2(1, 2).Opposite", "Vector2(-1, -2)"),
            TestCase("Vector2(-1, 2).Opposite", "Vector2(1, -2)"),
            TestCase("Vector2(1, -2).Opposite", "Vector2(-1, 2)"),
            TestCase("Vector2(-1, -2).Opposite", "Vector2(1, 2)"),
        ]
        [
            TestCase("Vector2(0, 0).Normalize", "Vector2(Num.NaN, Num.NaN)"),
            TestCase("Vector2(1, 1).Normalize", "Vector2(0.707106769, 0.707106769)"),
            TestCase("Vector2(3, 4).Normalize", "Vector2(0.6, 0.8)"),
        ]  
        [
            TestCase("Vector2(2, 4).Mul(0)", "Vector2(0, 0)"),
            TestCase("Vector2(2, 4).Mul(2)", "Vector2(4, 8)"),
            TestCase("Vector2(2, 4).Mul(0.5)", "Vector2(1, 2)"),
            TestCase("Vector2(2, 4).Mul(-2)", "Vector2(-4, -8)"),
            TestCase("Vector2(2, 4).Mul(-0.5)", "Vector2(-1, -2)"),
            TestCase("Vector2(-2, 4).Mul(2)", "Vector2(-4, 8)"),
            TestCase("Vector2(-2, 4).Mul(0.5)", "Vector2(-1, 2)"),
            TestCase("Vector2(-2, 4).Mul(-2)", "Vector2(4, -8)"),
            TestCase("Vector2(-2, 4).Mul(-0.5)", "Vector2(1, -2)"),
            TestCase("Vector2(2, -4).Mul(2)", "Vector2(4, -8)"),
            TestCase("Vector2(2, -4).Mul(0.5)", "Vector2(1, -2)"),
            TestCase("Vector2(2, -4).Mul(-2)", "Vector2(-4, 8)"),
            TestCase("Vector2(2, -4).Mul(-0.5)", "Vector2(-1, 2)"),
            TestCase("Vector2(-2, -4).Mul(2)", "Vector2(-4, -8)"),
            TestCase("Vector2(-2, -4).Mul(0.5)", "Vector2(-1, -2)"),
            TestCase("Vector2(-2, -4).Mul(-2)", "Vector2(4, 8)"),
            TestCase("Vector2(-2, -4).Mul(-0.5)", "Vector2(1, 2)"),
        ]        
        [
            TestCase("Vector2(2, 4).Div(0)", "Vector2(Num.PositiveInfinity, Num.PositiveInfinity)"),
            TestCase("Vector2(2, 4).Div(-0)", "Vector2(Num.NegativeInfinity, Num.NegativeInfinity)"),
            TestCase("Vector2(2, 4).Div(2)", "Vector2(1, 2)"),
            TestCase("Vector2(2, 4).Div(0.5)", "Vector2(4, 8)"),
            TestCase("Vector2(2, 4).Div(-2)", "Vector2(-1, -2)"),
            TestCase("Vector2(2, 4).Div(-0.5)", "Vector2(-4, -8)"),
            TestCase("Vector2(-2, 4).Div(2)", "Vector2(-1, 2)"),
            TestCase("Vector2(-2, 4).Div(0.5)", "Vector2(-4, 8)"),
            TestCase("Vector2(-2, 4).Div(-2)", "Vector2(1, -2)"),
            TestCase("Vector2(-2, 4).Div(-0.5)", "Vector2(4, -8)"),
            TestCase("Vector2(2, -4).Div(2)", "Vector2(1, -2)"),
            TestCase("Vector2(2, -4).Div(0.5)", "Vector2(4, -8)"),
            TestCase("Vector2(2, -4).Div(-2)", "Vector2(-1, 2)"),
            TestCase("Vector2(2, -4).Div(-0.5)", "Vector2(-4, 8)"),
            TestCase("Vector2(-2, -4).Div(2)", "Vector2(-1, -2)"),
            TestCase("Vector2(-2, -4).Div(0.5)", "Vector2(-4, -8)"),
            TestCase("Vector2(-2, -4).Div(-2)", "Vector2(1, 2)"),
            TestCase("Vector2(-2, -4).Div(-0.5)", "Vector2(4, 8)"),
        ]
        [
            TestCase("Vector2(0, 0).Add(Vector2(0, 0))", "Vector2(0, 0)"),
            TestCase("Vector2(1, 2).Add(Vector2(2, 1))", "Vector2(3, 3)"),
            TestCase("Vector2(1, 2).Add(Vector2(-2, -1))", "Vector2(-1, 1)"),
        ]        
        [
            TestCase("Vector2(0, 0).Sub(Vector2(0, 0))", "Vector2(0, 0)"),
            TestCase("Vector2(1, 2).Sub(Vector2(2, 1))", "Vector2(-1, 1)"),
            TestCase("Vector2(1, 2).Sub(Vector2(-2, -1))", "Vector2(3, 3)"),
        ]
        [
            TestCase("Vector2(0, 0).Dot(Vector2(0, 0))", "0"),
            TestCase("Vector2(1, 1).Dot(Vector2(0, 0))", "0"),
            TestCase("Vector2(1, 3).Dot(Vector2(2, 5))", "17"),
            TestCase("Vector2(1, 3).Dot(Vector2(-2, -5))", "-17"),
        ]
        [
            TestCase("Vector2(0, 0).Distance(Vector2(0, 0))", "0"),
            TestCase("Vector2(0, 0).Distance(Vector2(3, 4))", "5"),
        ]
        [
            TestCase("Vector2(0, 0).Angle(Vector2(0, 0))", "Num.NaN"),
            TestCase("Vector2(0, 1).Angle(Vector2(0, 1))", "0"),
            TestCase("Vector2(0, 1).Angle(Vector2(1, 1))", "45"),
            TestCase("Vector2(0, 1).Angle(Vector2(-1, 1))", "45"),
            TestCase("Vector2(0, 1).Angle(Vector2(1, -1))", "135"),
            TestCase("Vector2(0, 1).Angle(Vector2(-1, -1))", "135"),
        ]
        [
            TestCase("Vector2(1, 1).Reflect(Vector2(0, 1))", "Vector2(-1, 1)"),
            TestCase("Vector2(-1, 1).Reflect(Vector2(0, 1))", "Vector2(1, 1)"),
            TestCase("Vector2(1, -1).Reflect(Vector2(0, 1))", "Vector2(-1, -1)"),
            TestCase("Vector2(-1, -1).Reflect(Vector2(0, 1))", "Vector2(1, -1)"),
            TestCase("Vector2(1, 1).Reflect(Vector2(1, 0))", "Vector2(1, -1)"),
            TestCase("Vector2(-1, 1).Reflect(Vector2(1, 0))", "Vector2(-1, -1)"),
            TestCase("Vector2(1, -1).Reflect(Vector2(1, 0))", "Vector2(1, 1)"),
            TestCase("Vector2(-1, -1).Reflect(Vector2(1, 0))", "Vector2(-1, 1)"),
        ]
        public void Operations(string expression, string expected) => AssertApproxEqual(ValidatedCompilationInput, expression, expected);
        
        // [
        //     TestCase("Vector2(1, 3).Dot(Vector2(2, 5))", "17"),
        // ]
        // public void Operations(string expression, string expected) => AssertApproxEqual(ValidatedCompilationInput, expression, expected);
    }
}


/*

        [
            TestCase("Bool.not(true)", "false"),
            TestCase("Bool.not(false)", "true"),
        ]
        [
            TestCase("false.and(false)", "false"),
            TestCase("false.and(true)", "false"),
            TestCase("true.and(false)", "false"),
            TestCase("true.and(true)", "true")
        ]
        [
            TestCase("false.or(false)", "false"),
            TestCase("false.or(true)", "true"),
            TestCase("true.or(false)", "true"),
            TestCase("true.or(true)", "true")
        ]
        [
            TestCase("false.xor(false)", "false"),
            TestCase("false.xor(true)", "true"),
            TestCase("true.xor(false)", "true"),
            TestCase("true.xor(true)", "false")
        ]
        [
            TestCase("0.lt(0)", "false"),
            TestCase("0.2.lt(0)", "false"),
            TestCase("-0.2.lt(0)", "true"),
            TestCase("1.lt(2)", "true")
        ]
        [
            TestCase("0.gt(0)", "false"),
            TestCase("0.2.gt(0)", "true"),
            TestCase("-0.2.gt(0)", "false"),
            TestCase("2.gt(1)", "true")
        ]
        [
            TestCase("0.leq(0)", "true"),
            TestCase("0.2.leq(0)", "false"),
            TestCase("-0.2.leq(0)", "true"),
            TestCase("1.leq(2)", "true")
        ]
        [
            TestCase("0.geq(0)", "true"),
            TestCase("0.2.geq(0)", "true"),
            TestCase("-0.2.geq(0)", "false"),
            TestCase("2.geq(1)", "true")
        ]
        [
            TestCase("0.eq(0)", "true"),
            TestCase("1.eq(0)", "false"),
            TestCase("0.1.eq(0)", "false"),
            TestCase("999.999.eq(999.999)", "true")
        ]
        [
            TestCase("0.neq(0)", "false"),
            TestCase("1.neq(0)", "true"),
            TestCase("0.1.neq(0)", "true"),
            TestCase("999.999.neq(999.999)", "false")
        ]*/