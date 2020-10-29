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
            TestCase("Matrix4x4.identity.mulVec(Vector4(1, 1, 1, 0))", "Vector4(1, 1, 1, 0)"),
        ]
        public void MulVec(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);
    }
}
