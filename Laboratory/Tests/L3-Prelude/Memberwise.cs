using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
    internal class Memberwise : PreludeFixture
    {
        public Memberwise() :base("Memberwise") {}

        [Test]
        public void FirstArgMustBeFunction() => EvaluateExpectingErrorCode(ValidatedCompilationInput, 8, "memberwise(5)");
        
        [Test]
        public void MemberwiseVectorMul() => AssertApproxEqual(ValidatedCompilationInput, "Vector3(144, 24, -125)", "Vector3(12, -8, 5).mul(Vector3(12, -3, -25))");
    }
}