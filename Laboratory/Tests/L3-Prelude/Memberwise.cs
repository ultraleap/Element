using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
    internal class Memberwise : PreludeFixture
    {
        [Test]
        public void FirstArgMustBeFunction() => EvaluateExpectingErrorCode(ValidatedCompilerInput, MessageCode.ConstraintNotSatisfied, "memberwise(5)");
        
        [Test]
        public void MemberwiseVectorMul() => AssertApproxEqual(ValidatedCompilerInput, "Vector3(144, 24, -125)", "Vector3(12, -8, 5).mul(Vector3(12, -3, -25))");
    }
}