using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Matrix : StandardLibraryFixture
    {
        [
            TestCase("Matrix4x4.identity", 
                "Matrix4x4(" +
                "Vector4(1, 0, 0, 0), " +
                "Vector4(0, 1, 0, 0), " +
                "Vector4(0, 0, 1, 0), " +
                "Vector4(0, 0, 0, 1))"),
        ]
        public void CheckIdentity(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase(
            "Matrix4x4(" +
            "Vector4(1, 2, 3, 4)," +
            "Vector4(5, 6, 7, 8)," +
            "Vector4(9, 10, 11, 12)," +
            "Vector4(13, 14, 15, 16))" +
            ".xRow",
            "Vector4(1, 2, 3, 4)"
            ),
            TestCase(
            "Matrix4x4(" +
            "Vector4(1, 2, 3, 4)," +
            "Vector4(5, 6, 7, 8)," +
            "Vector4(9, 10, 11, 12)," +
            "Vector4(13, 14, 15, 16))" +
            ".yRow",
            "Vector4(5, 6, 7, 8)"
            ),
            TestCase(
            "Matrix4x4(" +
            "Vector4(1, 2, 3, 4)," +
            "Vector4(5, 6, 7, 8)," +
            "Vector4(9, 10, 11, 12)," +
            "Vector4(13, 14, 15, 16))" +
            ".zRow",
            "Vector4(9, 10, 11, 12)"
            ),
            TestCase(
            "Matrix4x4(" +
            "Vector4(1, 2, 3, 4)," +
            "Vector4(5, 6, 7, 8)," +
            "Vector4(9, 10, 11, 12)," +
            "Vector4(13, 14, 15, 16))" +
            ".wRow",
            "Vector4(13, 14, 15, 16)"
            ),
            TestCase(
            "Matrix4x4(" +
            "Vector4(1, 2, 3, 4)," +
            "Vector4(5, 6, 7, 8)," +
            "Vector4(9, 10, 11, 12)," +
            "Vector4(13, 14, 15, 16))" +
            ".xCol",
            "Vector4(1, 5, 9, 13)"
            ),
            TestCase(
            "Matrix4x4(" +
            "Vector4(1, 2, 3, 4)," +
            "Vector4(5, 6, 7, 8)," +
            "Vector4(9, 10, 11, 12)," +
            "Vector4(13, 14, 15, 16))" +
            ".yCol",
            "Vector4(2, 6, 10, 14)"
            ),
            TestCase(
            "Matrix4x4(" +
            "Vector4(1, 2, 3, 4)," +
            "Vector4(5, 6, 7, 8)," +
            "Vector4(9, 10, 11, 12)," +
            "Vector4(13, 14, 15, 16))" +
            ".zCol",
            "Vector4(3, 7, 11, 15)"
            ),
            TestCase(
            "Matrix4x4(" +
            "Vector4(1, 2, 3, 4)," +
            "Vector4(5, 6, 7, 8)," +
            "Vector4(9, 10, 11, 12)," +
            "Vector4(13, 14, 15, 16))" +
            ".wCol",
            "Vector4(4, 8, 12, 16)"
            ),
        ]
        public void GetRowsAndCols(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
           TestCase(
               "Matrix4x4.fromRows(" +
               "Vector4(1, 2, 3, 4), " +
               "Vector4(5, 6, 7, 8), " +
               "Vector4(9, 10, 11, 12), " +
               "Vector4(13, 14, 15, 16))",
               "Matrix4x4(" +
               "Vector4(1, 2, 3, 4), " +
               "Vector4(5, 6, 7, 8), " +
               "Vector4(9, 10, 11, 12), " +
               "Vector4(13, 14, 15, 16))"
           ),
            TestCase(
                "Matrix4x4.fromCols(" +
                "Vector4(1, 2, 3, 4), " +
                "Vector4(5, 6, 7, 8), " +
                "Vector4(9, 10, 11, 12), " +
                "Vector4(13, 14, 15, 16))",
                "Matrix4x4.fromRows(" +
                "Vector4(1, 2, 3, 4), " +
                "Vector4(5, 6, 7, 8), " +
                "Vector4(9, 10, 11, 12), " +
                "Vector4(13, 14, 15, 16)).transpose"
            ),
            TestCase(
                "Matrix4x4.fromDiagonal(Vector4(1, 2, 3, 4))",
                "Matrix4x4.fromRows(" +
                "Vector4(1, 0, 0, 0)," +
                "Vector4(0, 2, 0, 0)," +
                "Vector4(0, 0, 3, 0)," +
                "Vector4(0, 0, 0, 4))"
            )
       ]
        public void Constructors(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("Matrix4x4(" +
                 "Vector4(2, 0, 1, 4), " +
                 "Vector4(1, 0, 6, 3), " +
                 "Vector4(1, 1, 3, 3), " +
                 "Vector4(0, 2, 9, 1)).transpose", 
                "Matrix4x4(" +
                 "Vector4(2, 1, 1, 0), " +
                 "Vector4(0, 0, 1, 2), " +
                 "Vector4(1, 6, 3, 9), " +
                 "Vector4(4, 3, 3, 1))"),
        ]
        public void Transpose(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);



        [
            TestCase(
                "Matrix4x4.fromRows(" +
                "Vector4(1, 2, 3, 4), " +
                "Vector4(5, 6, 7, 8), " +
                "Vector4(9, 10, 11, 12), " +
                "Vector4(13, 14, 15, 16)).diagonal",
                "Vector4(1, 6, 11, 16)"
            )
        ]
        public void GetDiagonal(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase("Matrix4x4.identity.determinant", "1"),
            TestCase(
                "Matrix4x4.fromRows(" +
                "Vector4(1, 2, 3, 4), " +
                "Vector4(5, 6, 7, 8), " +
                "Vector4(9, 10, 11, 12), " +
                "Vector4(13, 14, 15, 16))" +
                ".determinant",
                "0"
            ),
            TestCase(
                "Matrix4x4.fromRows(" +
                "Vector4(4, 4, 32, 41), " +
                "Vector4(1, 6, 7, -10), " +
                "Vector4(9, 10, 11, 12), " +
                "Vector4(13, 14, 15, 16))" +
                ".determinant",
                "2320"
            ),
        ]
        public void Determinant(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase("Matrix4x4.identity.mul(Matrix4x4.identity)", "Matrix4x4.identity"),
            TestCase(
                "Matrix4x4.fromRows(" +
                "Vector4(1, 2, 3, 4), " +
                "Vector4(5, 6, 7, 8), " +
                "Vector4(9, 10, 11, 12), " +
                "Vector4(13, 14, 15, 16))" +
                ".mul(" +
                "Matrix4x4.fromRows(" +
                "Vector4(1, 2, 3, 4), " +
                "Vector4(5, 6, 7, 8), " +
                "Vector4(9, 10, 11, 12), " +
                "Vector4(13, 14, 15, 16)).transpose" +
                ")",
                "Matrix4x4.fromRows(" +
                "Vector4(30, 70, 110, 150), " +
                "Vector4(70, 174, 278, 382), " +
                "Vector4(110, 278, 446, 614), " +
                "Vector4(150, 382, 614, 846))"
            ),
        ]
        public void Multiply(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("Matrix4x4.identity.vectorMul(Vector4(1, 1, 1, 0))", "Vector4(1, 1, 1, 0)"),
            TestCase("Matrix4x4.identity.vectorMul(Vector4(1, 2, 3, 4))", "Vector4(1, 2, 3, 4)"),
            TestCase(
                "Matrix4x4.fromRows(" +
                "Vector4(1, 2, 3, 4), " +
                "Vector4(5, 6, 7, 8), " +
                "Vector4(9, 10, 11, 12), " +
                "Vector4(13, 14, 15, 16))" +
                ".vectorMul(Vector4(1, 2, 3, 4))",
                "Vector4(30, 70, 110, 150)"
            ),
        ]
        public void VectorMul(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
    }
}
