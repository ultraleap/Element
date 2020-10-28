using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Vector3 : StandardLibraryFixture
    {
        [
            TestCase("Vector3.zero", "Vector3(0, 0, 0)"),
            TestCase("Vector3.one", "Vector3(1, 1, 1)"),
            TestCase("Vector3.up", "Vector3(0, 0, 1)"),
            TestCase("Vector3.down", "Vector3(0, 0, -1)"),
            TestCase("Vector3.right", "Vector3(1, 0, 0)"),
            TestCase("Vector3.left", "Vector3(-1, 0, 0)"),
            TestCase("Vector3.forward", "Vector3(0, 1, 0)"),
            TestCase("Vector3.back", "Vector3(0, -1, 0)"),
        ]
        public void Constants(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector3(0, 0, 0).magnitudeSquared", "0"),
            TestCase("Vector3(1, 0, 0).magnitudeSquared", "1"),
            TestCase("Vector3(2, 3, 6).magnitudeSquared", "49"),
        ]
        public void MagnitudeSquared(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector3(0, 0, 0).magnitude", "0"),
            TestCase("Vector3(1, 0, 0).magnitude", "1"),
            TestCase("Vector3(1, 2, 2).magnitude", "3"),
        ]
        public void Magnitude(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector3(0, 0, 0).negate", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 2, 3).negate", "Vector3(-1, -2, -3)"),
            TestCase("Vector3(-1, 2, -3).negate", "Vector3(1, -2, 3)"),
            TestCase("Vector3(1, -2, 3).negate", "Vector3(-1, 2, -3)"),
            TestCase("Vector3(-1, -2, -3).negate", "Vector3(1, 2, 3)"),
        ]
        public void Negate(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector3(0, 0, 0).normalise", "Vector3(Num.NaN, Num.NaN, Num.NaN)"),
            TestCase("Vector3(1, 1, 1).normalise", "Vector3(1.div(3.sqrt), 1.div(3.sqrt), 1.div(3.sqrt))"),
            TestCase("Vector3(1, 2, 2).normalise", "Vector3(1.div(3), 2.div(3), 2.div(3))"),
        ]
        public void Normalize(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
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
        public void ScalarMultiply(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
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
        public void ScalarDivide(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector3(0, 0, 0).add(Vector3(0, 0, 0))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 2, 3).add(Vector3(0, 0, 0))", "Vector3(1, 2, 3)"),
            TestCase("Vector3(1, 2, 3).add(Vector3(2, 1, 3))", "Vector3(3, 3, 6)"),
            TestCase("Vector3(1, 2, 3).add(Vector3(-2, -1, -3))", "Vector3(-1, 1, 0)"),
        ]
        public void VectorAdd(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector3(0, 0, 0).sub(Vector3(0, 0, 0))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 2, 3).sub(Vector3(0, 0, 0))", "Vector3(1, 2, 3)"),
            TestCase("Vector3(1, 2, 3).sub(Vector3(2, 1, 3))", "Vector3(-1, 1, 0)"),
            TestCase("Vector3(1, 2, 3).sub(Vector3(-2, -1, -3))", "Vector3(3, 3, 6)"),
        ]
        public void VectorSubtract(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector3(0, 0, 0).dot(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(1, 1, 1).dot(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(1, 3, 7).dot(Vector3(2, 5, 4))", "45"),
            TestCase("Vector3(1, 3, 7).dot(Vector3(-2, -5, -4))", "-45"),
        ]
        public void DotProduct(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector3(0, 0, 0).distance(Vector3(0, 0, 0))", "0"),
            TestCase("Vector3(0, 0, 0).distance(Vector3(2, 3, 6))", "7"),
        ]
        public void Distance(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector3(0, 0, 0).angle(Vector3(0, 0, 0))", "Num.NaN"),
            TestCase("Vector3(0, 1, 1).angle(Vector3(0, 1, 1))", "0"),
            TestCase("Vector3(0, 1, 1).angle(Vector3(1, 1, 0))", "60"),
            TestCase("Vector3(0, 1, 1).angle(Vector3(-1, 0, 1))", "60"),
            TestCase("Vector3(0, 1, 1).angle(Vector3(1, -1, 1))", "90"),
            TestCase("Vector3(0, 1, 1).angle(Vector3(-1, -1, 0))", "120"),
        ]
        public void Angle(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
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
        public void Reflect(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Vector3(0, 0, 0).cross(Vector3(0, 0, 0))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 1, 1).cross(Vector3(1, 1, 1))", "Vector3(0, 0, 0)"),
            TestCase("Vector3(1, 1, 1).cross(Vector3(1, 1, 0))", "Vector3(-1, 1, 0)"),
            TestCase("Vector3(3, 4, 5).cross(Vector3(1, 0, 0))", "Vector3(0, 5, -4)"),
        ]
        public void CrossProduct(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
                
        [
            TestCase("Vector3.lerp(-0.25, Vector3.zero, Vector3.one)", "Vector3(-0.25, -0.25, -0.25)"), //extrapolation
            TestCase("Vector3.lerp(0,     Vector3.zero, Vector3.one)", "Vector3(0, 0, 0)"),
            TestCase("Vector3.lerp(0.25,  Vector3.zero, Vector3.one)", "Vector3(0.25, 0.25, 0.25)"),
            TestCase("Vector3.lerp(0.5,   Vector3.zero, Vector3.one)", "Vector3(0.5, 0.5, 0.5)"),
            TestCase("Vector3.lerp(0.75,  Vector3.zero, Vector3.one)", "Vector3(0.75, 0.75, 0.75)"),
            TestCase("Vector3.lerp(1,     Vector3.zero, Vector3.one)", "Vector3(1, 1, 1)"),
            TestCase("Vector3.lerp(1.25,  Vector3.zero, Vector3.one)", "Vector3(1.25, 1.25, 1.25)"), //extrapolation
        ]
        public void LinearInterpolation(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
    }
}