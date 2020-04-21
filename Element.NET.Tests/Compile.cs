using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using Element.CLR;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class Compile : FixtureBase
    {
        private string CompileSource = 
@"struct Vector3(x:Num, y:Num, z:Num)
{
    add(a:Vector3, b:Vector3) = memberwise(Num.add, a, b);
}";
        
        private class MyCustomElementStruct
        {
            public float floatField;
            public Vector3 vector3Field;
        }
        
        private delegate float InvalidDelegate(object obj);
        private delegate float Constant();
        private delegate float UnaryOp(float a);
        private delegate float BinaryOp(float a, float b);
        private delegate float IndexArray(List<float> list, float item);
        private delegate Vector3 VectorOperation(Vector3 a, Vector3 b);

        private delegate MyCustomElementStruct CustomStructDelegate(float f, Vector3 v3);

        private SourceContext _sourceContext => MakeSourceContext(extraSource: CompileSource);
        
        [Test]
        public void ConstantPi()
        {
            var pi = _sourceContext.Compile<Constant>("Num.pi");
            Assert.AreEqual(3.14159265359f, pi());
        }
        
        [Test]
        public void BinaryAdd()
        {
            var add = _sourceContext.Compile<BinaryOp>("Num.add");
            Assert.AreEqual(11f, add(3f, 8f));
        }

        [Test]
        public void ListWithStaticCount() => Assert.AreEqual(20f, _sourceContext.Compile<Constant>("List.repeat(4, 5).fold(0, Num.add)")());

        private static readonly int[] _dynamicListCounts = Enumerable.Repeat(1, 20).ToArray();

        [TestCaseSource(nameof(_dynamicListCounts))]
        public void ListWithDynamicCount(int count)
        {
            var dynamicList = _sourceContext.Compile<UnaryOp>("_(a:Num):Num = List.repeat(1, a).fold(0, Num.add)");
            Assert.AreEqual(count, dynamicList(count));
        }

        [Test]
        public void NoObjectBoundaryConverter()
        {
            var shouldBeNull = _sourceContext.Compile<InvalidDelegate>("Num.sqr");
            Assert.Null(shouldBeNull);
            // TODO: Expect InvalidBoundaryFunction error code (ELE10)
        }

        [Test]
        public void TopLevelList()
        {
            var fn = _sourceContext.Compile<IndexArray>("_(list:List, idx:Num):Num = list.at(idx)");
            var thirdElement = fn(new List<float> {1f, 4f, 7f, -3f}, 3);
            Assert.AreEqual(7f, thirdElement);
        }

        [Test]
        public void IntermediateStructVectorAdd()
        {
            var fn = _sourceContext.Compile<BinaryOp>("_(a:Num, b:Num):Num = Vector3(a, a, a).add(Vector3(b, b, b))");
            var result = fn(5f, 10f);
            Assert.AreEqual(15f, result);
        }
        
        [Test]
        public void StructVectorAdd()
        {
            var fn = _sourceContext.Compile<VectorOperation>("Vector3.add");
            var result = fn(new Vector3(5f), new Vector3(10f));
            Assert.AreEqual(15f, result.X);
        }

        [Test]
        public void MakeCustomStructInstance()
        {
            var fn = _sourceContext.Compile<CustomStructDelegate>("_(f:Num, v3:Vector3):MyCustomElementStruct = MyCustomElementStruct(f, v3)");
            var result = fn(5f, new Vector3(10f));
            Assert.AreEqual(5f, result.floatField);
            Assert.AreEqual(10f, result.vector3Field.X);
        }
    }
}