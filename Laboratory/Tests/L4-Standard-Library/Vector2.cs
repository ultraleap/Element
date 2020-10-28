using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Vector2 : StandardLibraryFixture
    {
        [
            TestCase("Vector2.zero", "Vector2(0, 0)"),
            TestCase("Vector2.one", "Vector2(1, 1)"),
            TestCase("Vector2.right", "Vector2(1, 0)"),
            TestCase("Vector2.left", "Vector2(-1, 0)"),
            TestCase("Vector2.up", "Vector2(0, 1)"),
            TestCase("Vector2.down", "Vector2(0, -1)"),
        ]
        public void Constants(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector2(0, 0).magnitudeSquared", "0"),
            TestCase("Vector2(1, 0).magnitudeSquared", "1"),
            TestCase("Vector2(3, 4).magnitudeSquared", "25"),
        ]
         public void MagnitudeSquared(string expression, string expected) =>
             AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector2(0, 0).magnitude", "0"),
            TestCase("Vector2(1, 0).magnitude", "1"),
            TestCase("Vector2(3, 4).magnitude", "5"),
        ]
        public void Magnitude(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector2(0, 0).opposite", "Vector2(0, 0)"),
            TestCase("Vector2(1, 2).opposite", "Vector2(-1, -2)"),
            TestCase("Vector2(-1, 2).opposite", "Vector2(1, -2)"),
            TestCase("Vector2(1, -2).opposite", "Vector2(-1, 2)"),
            TestCase("Vector2(-1, -2).opposite", "Vector2(1, 2)"),
        ]
        public void Opposite(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector2(0, 0).normalise", "Vector2(Num.NaN, Num.NaN)"),
            TestCase("Vector2(1, 1).normalise", "Vector2(1.div(2.sqrt), 1.div(2.sqrt))"),
            TestCase("Vector2(3, 4).normalise", "Vector2(3.div(5), 4.div(5))"),
        ]
        public void Normalize(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
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
        public void ScalarMultiply(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
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
        public void ScalarDivide(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector2(0, 0).add(Vector2(0, 0))", "Vector2(0, 0)"),
            TestCase("Vector2(1, 2).add(Vector2(0, 0))", "Vector2(1, 2)"),
            TestCase("Vector2(1, 2).add(Vector2(2, 1))", "Vector2(3, 3)"),
            TestCase("Vector2(1, 2).add(Vector2(-2, -1))", "Vector2(-1, 1)"),
        ]
        public void VectorAdd(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector2(0, 0).sub(Vector2(0, 0))", "Vector2(0, 0)"),
            TestCase("Vector2(1, 2).sub(Vector2(0, 0))", "Vector2(1, 2)"),
            TestCase("Vector2(1, 2).sub(Vector2(2, 1))", "Vector2(-1, 1)"),
            TestCase("Vector2(1, 2).sub(Vector2(-2, -1))", "Vector2(3, 3)"),
        ]
        public void VectorSubtract(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector2(0, 0).dot(Vector2(0, 0))", "0"),
            TestCase("Vector2(1, 1).dot(Vector2(0, 0))", "0"),
            TestCase("Vector2(1, 3).dot(Vector2(2, 5))", "17"),
            TestCase("Vector2(1, 3).dot(Vector2(-2, -5))", "-17"),
        ]
        public void DotProduct(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector2(0, 0).distance(Vector2(0, 0))", "0"),
            TestCase("Vector2(0, 0).distance(Vector2(3, 4))", "5"),
        ]
        public void Distance(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector2(0, 0).angle(Vector2(0, 0))", "Num.NaN"),
            TestCase("Vector2(0, 1).angle(Vector2(0, 1))", "0"),
            TestCase("Vector2(0, 1).angle(Vector2(1, 1))", "45"),
            TestCase("Vector2(0, 1).angle(Vector2(-1, 1))", "45"),
            TestCase("Vector2(0, 1).angle(Vector2(1, -1))", "135"),
            TestCase("Vector2(0, 1).angle(Vector2(-1, -1))", "135"),
        ]
        public void Angle(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
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
        public void Reflect(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
                
        [
            TestCase("Vector2.lerp(-0.25, Vector2.zero, Vector2.one)", "Vector2(-0.25, -0.25)"), //extrapolation
            TestCase("Vector2.lerp(0,     Vector2.zero, Vector2.one)", "Vector2(0, 0)"),
            TestCase("Vector2.lerp(0.25,  Vector2.zero, Vector2.one)", "Vector2(0.25, 0.25)"),
            TestCase("Vector2.lerp(0.5,   Vector2.zero, Vector2.one)", "Vector2(0.5, 0.5)"),
            TestCase("Vector2.lerp(0.75,  Vector2.zero, Vector2.one)", "Vector2(0.75, 0.75)"),
            TestCase("Vector2.lerp(1,     Vector2.zero, Vector2.one)", "Vector2(1, 1)"),
            TestCase("Vector2.lerp(1.25,  Vector2.zero, Vector2.one)", "Vector2(1.25, 1.25)"), //extrapolation
        ]
        public void LinearInterpolation(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
    }
}