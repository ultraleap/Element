using Element.CLR;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class Compile : FixtureBase
    {
        private delegate float Constant();
        private delegate float UnaryOp(float a);
        private delegate float BinaryOp(float a, float b);
        
        

        [Test]
        public void ConstantPi()
        {
            var sourceContext = MakeSourceContext();
            var pi = sourceContext.Compile<Constant>("Num.pi");
            Assert.AreEqual(3.14159265359f, pi());
        }
        
        [Test]
        public void BinaryAdd()
        {
            var sourceContext = MakeSourceContext();
            var add = sourceContext.Compile<BinaryOp>("Num.add");
            Assert.AreEqual(11f, add(3f, 8f));
        }
        
    }
}