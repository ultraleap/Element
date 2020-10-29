using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Transform : StandardLibraryFixture
    {
        [
            TestCase("Transform(Matrix4x4.fromCols(" +
                     "Vector4(2, 0, 1, 4), " +
                     "Vector4(1, 0, 6, 3), " +
                     "Vector4(1, 1, 3, 3), " +
                     "Vector4(0, 2, 9, 1))).translation",
                "Vector3(0, 2, 9)"),
        ]
        public void Position(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("Transform.fromAxisAngle(Vector3(0, 0, 1), Num.pi.div(2)).applyToDirection(Vector3(1, 1, 0))", "Vector3(-1, 1, 0)")
        ]
        public void FromAxisAngle(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
    }
}