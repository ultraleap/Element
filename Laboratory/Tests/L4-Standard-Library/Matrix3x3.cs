using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Matrix3x3 : StandardLibraryFixture
    {


        [
            TestCase
            (
            "Matrix3x3(" +
                "Vector3(1, 2, 3), " +
                "Vector3(4, 5, 6), " +
                "Vector3(7, 8, 9)" +
                ")",
            "Matrix3x3.fromRows(" +
                "Vector3(1, 2, 3), " +
                "Vector3(4, 5, 6), " +
                "Vector3(7, 8, 9)" +
                ")"
            ),
            TestCase
            (
            "Matrix3x3(" +
                "Vector3(1, 2, 3), " +
                "Vector3(4, 5, 6), " +
                "Vector3(7, 8, 9)" +
                ")",
            "Matrix3x3.fromCols(" +
                "Vector3(1, 4, 7), " +
                "Vector3(2, 5, 8), " +
                "Vector3(3, 6, 9)" +
                ")"
            ),

        ]
        public void Constructors(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);



        [
            TestCase(
            "Matrix3x3.identity",
            "Matrix3x3(" +
                "Vector3(1, 0, 0), " +
                "Vector3(0, 1, 0), " +
                "Vector3(0, 0, 1)" +
                ")"
            ),

        ]
        public void CheckIdentity(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase(
            "Matrix3x3.fromCols(" +
                "Vector3(1, 4, 7), " +
                "Vector3(2, 5, 8), " +
                "Vector3(3, 6, 9)" +
                ").xRow",
            "Vector3(1, 2, 3)"
            ),
            TestCase(
            "Matrix3x3.fromCols(" +
                "Vector3(1, 4, 7), " +
                "Vector3(2, 5, 8), " +
                "Vector3(3, 6, 9)" +
                ").yRow",
            "Vector3(4, 5, 6)"
            ),
            TestCase(
            "Matrix3x3.fromCols(" +
                "Vector3(1, 4, 7), " +
                "Vector3(2, 5, 8), " +
                "Vector3(3, 6, 9)" +
                ").zRow",
            "Vector3(7, 8, 9)"
            ),
            TestCase(
            "Matrix3x3.fromCols(" +
                "Vector3(1, 4, 7), " +
                "Vector3(2, 5, 8), " +
                "Vector3(3, 6, 9)" +
                ").diagonal",
            "Vector3(1, 5, 9)"
            ),
        ]
        public void CheckVectors(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase(
            "Matrix3x3.identity.determinant",
            "1"
            ),
            TestCase(
            "Matrix3x3.fromCols(" +
                "Vector3(1, 4, 7), " +
                "Vector3(2, 5, 8), " +
                "Vector3(3, 6, 9)" +
                ").determinant",
            "0"
            ),
            TestCase(
            "Matrix3x3.fromCols(" +
                "Vector3(2, 0, 0), " +
                "Vector3(0, 2, 0), " +
                "Vector3(0, 0, 2)" +
                ").determinant",
            "8"
            ),
        ]
        public void CheckDeterminant(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase(
            "Matrix3x3.identity" +
            ".mulVector(Vector3(1, 2, 3))",
            "Vector3(1, 2, 3)"
            ),
            TestCase(
            "Matrix3x3.fromCols(Vector3(1, 4, 7), Vector3(2, 5, 8), Vector3(3, 6, 9))" +
            ".mulVector(Vector3(1, 2, 3))",
            "Vector3(14, 32, 50)"
            ),
        ]
        public void CheckmulVector(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase(
            "Matrix3x3.identity" +
            ".mul(Matrix3x3.fromCols(Vector3(1, 4, 7), Vector3(2, 5, 8), Vector3(3, 6, 9)))",
            "Matrix3x3.fromCols(Vector3(1, 4, 7), Vector3(2, 5, 8), Vector3(3, 6, 9))"
            ),
            TestCase(
            "Matrix3x3.fromCols(Vector3(1, 4, 7), Vector3(2, 5, 8), Vector3(3, 6, 9))" +
            ".mul(Matrix3x3.fromCols(Vector3(9, 6, 3), Vector3(8, 5, 2), Vector3(7, 4, 1)))",
            "Matrix3x3.fromCols(Vector3(30, 84, 138), Vector3(24, 69, 114), Vector3(18, 54, 90))"
            ),
        ]
        public void CheckMul(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

    }
}
