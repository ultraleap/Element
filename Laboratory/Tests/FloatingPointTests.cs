using NUnit.Framework;

namespace Laboratory.Tests.Other
{
    [TestFixture, Parallelizable(ParallelScope.All)]
    internal class FloatingPointTests
    {
        public void NearlyEqual(float a, float b, bool expected) =>
            Assert.That(HostFixture.ApproximatelyEqualEpsilon(a, b, HostFixture.FloatEpsilon), Is.EqualTo(expected));

        public void NearlyEqual(float a, float b, float epsilon, bool expected) =>
            Assert.That(HostFixture.ApproximatelyEqualEpsilon(a, b, epsilon), Is.EqualTo(expected));

        //Test cases taken from https://floating-point-gui.de/errors/NearlyEqualsTest.java (https://floating-point-gui.de/errors/comparison/)
        [
            TestCase(1000000f, 1000001f, true),
            TestCase(1000001f, 1000000f, true),
            TestCase(10000f, 10001f, false),
            TestCase(10001f, 10000f, false),
        ]
        public void Big(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [
            TestCase(-1000000f, -1000001f, true),
            TestCase(-1000001f, -1000000f, true),
            TestCase(-10000f, -10001f, false),
            TestCase(-10001f, -10000f, false),
        ]
        public void BigNegative(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [
            TestCase(1.0000001f, 1.0000002f, true),
            TestCase(1.0000002f, 1.0000001f, true),
            TestCase(1.0002f, 1.0001f, false),
            TestCase(1.0001f, 1.0002f, false),
        ]
        public void Mid(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [
            TestCase(-1.000001f, -1.000002f, true),
            TestCase(-1.000002f, -1.000001f, true),
            TestCase(-1.0001f, -1.0002f, false),
            TestCase(-1.0002f, -1.0001f, false),
        ]
        public void MidNegative(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [
            TestCase(0.000000001000001f, 0.000000001000002f, true),
            TestCase(0.000000001000002f, 0.000000001000001f, true),
            TestCase(0.000000000001002f, 0.000000000001001f, false),
            TestCase(0.000000000001001f, 0.000000000001002f, false),
        ]
        public void Small(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [
            TestCase(-0.000000001000001f, -0.000000001000002f, true),
            TestCase(-0.000000001000002f, -0.000000001000001f, true),
            TestCase(-0.000000000001002f, -0.000000000001001f, false),
            TestCase(-0.000000000001001f, -0.000000000001002f, false),
        ]
        public void SmallNegative(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [
            TestCase(0.3f, 0.30000003f, true),
            TestCase(-0.3f, -0.30000003f, true),
        ]
        public void SmallDifferences(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [
            TestCase(0.0f, 0.0f, true),
            TestCase(-0.000000001000002f, -0.000000001000001f, true),
            TestCase(0.0f, -0.0f, true),
            TestCase(0.00000001f, 0.0f, false),
            TestCase(0.0f, 0.00000001f, false),
            TestCase(-0.00000001f, 0.0f, false),
            TestCase(0.0f, -0.00000001f, false),
        ]
        // [
        //     TestCase(0.0f, 4.371139e-08f, true), //(float)Math.Cos(90f*3.14159265359f/180f)
        //     TestCase(4.371139e-08f, 0.0f, true),
        //     TestCase(0.0f, -4.371139e-08f,true),
        //     TestCase(-4.371139e-08f, 0.0f,true),
        //     TestCase(0.0f, 8.742278e-08f, true), //(float)Math.Sin(180f*3.14159265359f/180f)
        //     TestCase(8.742278e-08f, 0.0f, true),
        //     TestCase(0.0f, -8.742278e-08f,true),
        //     TestCase(-8.742278e-08f, 0.0f,true),
        // ]
        public void Zero(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [
            TestCase(0.0f, 1e-40f, 0.01f, true),
            TestCase(1e-40f, 0.0f, 0.01f, true),
            TestCase(1e-40f, 0.0f, 0.000001f, false),
            TestCase(0.0f, 1e-40f, 0.000001f, false),
        ]
        [
            TestCase(0.0f, -1e-40f, 0.1f, true),
            TestCase(-1e-40f, 0.0f, 0.1f, true),
            TestCase(-1e-40f, 0.0f, 0.00000001f, false),
            TestCase(0.0f, -1e-40f, 0.00000001f, false),
        ]
        public void Zero(float a, float b, float epsilon, bool expected) => NearlyEqual(a, b, epsilon, expected);
        
        [
            TestCase(float.MaxValue, float.MaxValue, true),
            TestCase(float.MaxValue, -float.MaxValue, false),
            TestCase(-float.MaxValue, float.MaxValue, false),
            TestCase(float.MaxValue, float.MaxValue / 2, false),
            TestCase(float.MaxValue, -float.MaxValue / 2, false),
            TestCase(-float.MaxValue, float.MaxValue / 2, false),
        ]
        public void ExtremeMax(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [
            TestCase(float.PositiveInfinity, float.PositiveInfinity, true),
            TestCase(float.NegativeInfinity, float.NegativeInfinity, true),
            TestCase(float.NegativeInfinity, float.PositiveInfinity, false),
            TestCase(float.PositiveInfinity, float.MaxValue, false),
            TestCase(float.NegativeInfinity, -float.MaxValue, false),
        ]
        public void Infinities(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [
            //TestCase(float.NaN, float.NaN, false),
            TestCase(float.NaN, float.NaN, true), //we're intentionally setting this case to true for NaN comparisons to work e.g. (float)Math.Sqrt(-25) == float.NaN
            TestCase(float.NaN, 0.0f, false),
            TestCase(-0.0f, float.NaN, false),
            TestCase(float.NaN, -0.0f, false),
            TestCase(0.0f, float.NaN, false),
            TestCase(float.NaN, float.PositiveInfinity, false),
            TestCase(float.PositiveInfinity, float.NaN, false),
            TestCase(float.NaN, float.NegativeInfinity, false),
            TestCase(float.NegativeInfinity, float.NaN, false),
            TestCase(float.NaN, float.MaxValue, false),
            TestCase(float.MaxValue, float.NaN,  false),
            TestCase(float.NaN, -float.MaxValue, false),
            TestCase(-float.MaxValue, float.NaN,  false),
            TestCase(float.NaN, float.Epsilon, false),
            TestCase(float.Epsilon, float.NaN,  false),
            TestCase(float.NaN, -float.Epsilon, false),
            TestCase(-float.Epsilon, float.NaN,  false),
        ]
        public void NotNumber(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [   
            TestCase(1.000000001f, -1.0f,  false),
            TestCase(-1.0f, 1.000000001f, false),
            TestCase(-1.000000001f, 1.0f,  false),
            TestCase(1.0f, -1.000000001f,  false),
            TestCase(10 * float.Epsilon, 10 * -float.Epsilon,  true),
            TestCase(10000 * float.Epsilon, 10000 * -float.Epsilon,  false),
        ]
        public void Opposite(float a, float b, bool expected) => NearlyEqual(a, b, expected);
        
        [   
            TestCase(float.Epsilon, float.Epsilon,  true),
            TestCase(float.Epsilon, -float.Epsilon, true),
            TestCase(-float.Epsilon, float.Epsilon,  true),
            TestCase(float.Epsilon, 0,  true),
            TestCase(0, float.Epsilon,  true),
            TestCase(-float.Epsilon, 0,  true),
            TestCase(0, -float.Epsilon,  true),
        ]
        [   
            TestCase(0.000000001f, -float.Epsilon,  false),
            TestCase(0.000000001f, float.Epsilon, false),
            TestCase(float.Epsilon, 0.000000001f,  false),
            TestCase(-float.Epsilon, 0.000000001f,  false),
        ]
        public void Ulp(float a, float b, bool expected) => NearlyEqual(a, b, expected); //ulp? ultra-low precision?
    }
}