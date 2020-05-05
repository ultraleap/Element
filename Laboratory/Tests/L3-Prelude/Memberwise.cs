using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
    internal class Memberwise : PreludeFixture
    {
        [Test]
        public void FirstArgMustBeFunction() => EvaluateExpectingErrorCode(CompilationInput, 8, "memberwise(5)");
        
        [Test]
        public void MemberwiseVectorMul() => AssertApproxEqual(CompilationInput, "Vector3(144, 24, -125)", "Vector3(12, -8, 5).mul(Vector3(12, -3, -25))");
    }
}