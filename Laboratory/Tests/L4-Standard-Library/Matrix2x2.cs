using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Matrix2x2 : StandardLibraryFixture
    {
        [
            TestCase("Matrix2x2.identity",
                "Matrix2x2(" +
                "Vector2(1, 0), " +
                "Vector2(0, 1))"),
        ]
        public void CheckIdentity(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase("Matrix2x2.fromDiagonal(Vector2(3, 5))",
                "Matrix2x2(" +
                "Vector2(3, 0), " +
                "Vector2(0, 5))"),
        ]
        public void FromDiagonal(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("Matrix2x2(Vector2(1, 3),Vector2(2, 4)).xCol", "Vector2(1, 3)"),
            TestCase("Matrix2x2(Vector2(1, 3),Vector2(2, 4)).yCol", "Vector2(2, 4)"),
            TestCase("Matrix2x2(Vector2(1, 3),Vector2(2, 4)).xRow", "Vector2(1, 2)"),
            TestCase("Matrix2x2(Vector2(1, 3),Vector2(2, 4)).yRow", "Vector2(3, 4)"),
        ]
        public void GetRowsAndCols(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase("Matrix2x2(Vector2(1, 3),Vector2(2, 4)).diagonal", "Vector2(1, 4)"),
        ]
        public void GetDiagonal(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase("Matrix2x2.identity.determinant", "1"),
            TestCase("Matrix2x2(Vector2(1, 2), Vector2(3, 4)).determinant", "-2"),
            TestCase("Matrix2x2(Vector2(5, 1), Vector2(2, 5)).determinant", "23"),
            TestCase("Matrix2x2(Vector2(1, 1), Vector2(2, 1)).determinant", "-1"),
        ]
        public void Determinant(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase("Matrix2x2(Vector2(1, 3),Vector2(2, 4)).transpose", "Matrix2x2(Vector2(1, 2),Vector2(3, 4))"),
        ]
        public void Transpose(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase(
                "Matrix2x2.identity" +
                ".mulVec(Vector2(-5, 6))",
                "Vector2(-5, 6)"
            ),
            TestCase(
                "Matrix2x2(Vector2(3, 1), Vector2(-2, 4))" +
                ".mulVec(Vector2(-5, 6))",
                "Vector2(-27, 19)"
            ),
        ]
        public void MultiplyVector(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase(
                "Matrix2x2.identity" +
                ".mul(Matrix2x2.identity)",
                "Matrix2x2.identity"
            ),
            TestCase(
                "Matrix2x2(Vector2(1, 3), Vector2(2, 4))" +
                ".mul(Matrix2x2.identity)",
                "Matrix2x2(Vector2(1, 3), Vector2(2, 4))"
            ),
            TestCase(
                "Matrix2x2(Vector2(1, 3), Vector2(2, 4))" +
                ".mul(Matrix2x2(Vector2(-1, -5), Vector2(4, 3)))",
                "Matrix2x2(Vector2(-11, -23), Vector2(10, 24))"
            ),
        ]
        public void MultiplyMatrix(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
    }
}