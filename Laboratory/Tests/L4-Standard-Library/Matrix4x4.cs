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
            TestCase("Matrix4x4(" +
                     "Vector4(2, 0, 1, 4), " +
                     "Vector4(1, 0, 6, 3), " +
                     "Vector4(1, 1, 3, 3), " +
                     "Vector4(0, 2, 9, 1)).position", 
                "Vector3(0, 2, 9)"),
        ]
        public void Position(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
        
        [
            TestCase("Matrix4x4.identity.transform(Vector4(1, 1, 1, 0))", "Vector4(1, 1, 1, 0)"),
            TestCase("Matrix4x4.fromAxisAngle(Vector3(0, 0, 1), Num.pi.div(2)).transform(Vector4(1, 1, 0, 0))", "Vector4(-1, 1, 0, 0)")
        ]
        public void Transform(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
    }
}