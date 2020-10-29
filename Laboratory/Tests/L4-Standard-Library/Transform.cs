using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Transform : StandardLibraryFixture
    {

        [
            TestCase(
                "Transform.fromRotationAndTranslation(" +
                    "Matrix3x3.fromRows(" +
                    "Vector3(1, 2, 3)," + 
                    "Vector3(4, 5, 6)," + 
                    "Vector3(7, 8, 9)" +
                    ")" +
                "," +
                "Vector3(10, 11, 12)" +
                ")",
                "Transform(" +
                "Matrix4x4.fromRows(" +
                "Vector4(1, 2, 3, 10)," +
                "Vector4(4, 5, 6, 11)," +
                "Vector4(7, 8, 9, 12)," +
                "Vector4(0, 0, 0, 1)))"
            ),
            TestCase(
                "Transform.fromRotation(" +
                    "Matrix3x3.fromRows(" +
                    "Vector3(1, 2, 3)," +
                    "Vector3(4, 5, 6)," +
                    "Vector3(7, 8, 9)" +
                    ")" +
                ")",
                "Transform(" +
                "Matrix4x4.fromRows(" +
                "Vector4(1, 2, 3, 0)," +
                "Vector4(4, 5, 6, 0)," +
                "Vector4(7, 8, 9, 0)," +
                "Vector4(0, 0, 0, 1)))"
            ),
            TestCase(
                "Transform.fromTranslation(" +
                    "Vector3(10, 11, 12)" + 
                ")",
                "Transform(" +
                "Matrix4x4.fromRows(" +
                "Vector4(1, 0, 0, 10)," +
                "Vector4(0, 1, 0, 11)," +
                "Vector4(0, 0, 1, 12)," +
                "Vector4(0, 0, 0, 1)))"
            ),
        ]
        public void Constructors(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase(
                "Transform.fromTranslation(Vector3(1, 2, 3)).translation",
                "Vector3(1, 2, 3)"
            ),
            TestCase(
                "Transform.fromRotation(" +
                "Matrix3x3.fromRows(" +
                "Vector3(1, 2, 3)," +
                "Vector3(4, 5, 6)," +
                "Vector3(7, 8, 9))" +
                ").rotation",
                "Matrix3x3.fromRows(" +
                "Vector3(1, 2, 3)," +
                "Vector3(4, 5, 6)," +
                "Vector3(7, 8, 9))"
            ),
            TestCase("Transform(Matrix4x4.fromCols(" +
                     "Vector4(2, 0, 1, 4), " +
                     "Vector4(1, 0, 6, 3), " +
                     "Vector4(1, 1, 3, 3), " +
                     "Vector4(0, 2, 9, 1))).translation",
                "Vector3(0, 2, 9)"
            ),
        ]
        public void GetComponents(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase(
            "Transform.fromTranslation(Vector3.one).applyToDirection(Vector3(1, 2, 3))",
            "Vector3(1, 2, 3)"
            ),
            TestCase(
            "Transform.fromTranslation(Vector3.one).applyToPosition(Vector3(1, 2, 3))",
            "Vector3(2, 3, 4)"
            ),
            TestCase(
            "Transform.fromRotationAndTranslation(" +
            "Matrix3x3.fromRows(" +
            "Vector3(0, 1, 0)," +
            "Vector3(-1, 0, 0)," +
            "Vector3(0, 0, 1))," +
            "Vector3.one)" +
            ".applyToDirection(Vector3(1, 2, 3))",
            "Vector3(2, -1, 3)"
            ),
            TestCase(
            "Transform.fromRotationAndTranslation(" +
            "Matrix3x3.fromRows(" +
            "Vector3(0, 1, 0)," +
            "Vector3(-1, 0, 0)," +
            "Vector3(0, 0, 1))," +
            "Vector3.one)" +
            ".applyToPosition(Vector3(1, 2, 3))",
            "Vector3(3, 0, 4)"
            ),
        ]
        public void ApplyToVector3(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("Transform.fromAxisAngle(Vector3(0, 0, 1), Num.pi.div(2)).applyToDirection(Vector3(1, 1, 0))", "Vector3(-1, 1, 0)")
        ]
        public void FromAxisAngle(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
    }
}