using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using Element.AST;
using Element.CLR;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class Compile : FixtureBase
    {
        private const string _compileSource = @"struct Vector3(x:Num, y:Num, z:Num)
{
    add(a:Vector3, b:Vector3) = memberwise(Num.add, a, b);
}
struct MyCustomElementStruct(floatField:Num, vector3Field:Vector3);
struct CustomNestedStruct(structField:MyCustomElementStruct, floatField:Num, vector3Field:Vector3);
";

        [ElementStructTemplate("MyCustomElementStruct")]
        private struct MyCustomElementStruct
        {
            public float floatField;
            public Vector3 vector3Field;
        }
        
        [ElementStructTemplate("CustomNestedStruct")]
        private struct CustomNestedStruct
        {
            public MyCustomElementStruct structField;
            public float floatField;
            public Vector3 vector3Field;
        }
        
        private delegate float InvalidDelegate(object a);
        private delegate float Constant();
        private delegate float UnaryOp(float a);
        private delegate float BinaryOp(float a, float b);
        private delegate float IndexArray(List<float> list, float item);
        private delegate Vector3 VectorOperation(Vector3 a, Vector3 b);

        private delegate MyCustomElementStruct CustomStructDelegateNoArgs();
        private delegate MyCustomElementStruct CustomStructDelegate(float f, Vector3 v3);

        private TDelegate CompileAs<TDelegate>(SourceContext context, string expression, out float[] arguments) where TDelegate : Delegate
        {
            var function = context.EvaluateExpressionAs<IFunctionSignature>(expression, out var compilationContext);
            function.SourceArgumentsFromSerializedArray(out arguments, compilationContext);
            var (fn, _) = context.Compile<TDelegate>(function);
            return fn;
        }
        
        private TDelegate CompileAs<TDelegate>(SourceContext context, string expression) where TDelegate : Delegate
        {
            var function = context.EvaluateExpressionAs<IValue>(expression, out _);
            var (fn, _) = context.Compile<TDelegate>(function);
            return fn;
        }
        
        [Test]
        public void ConstantPi()
        {
            var pi = CompileAs<Constant>(MakeSourceContext(), "Num.pi");
            Assert.AreEqual(3.14159265359f, pi());
        }
        
        [Test]
        public void BinaryAdd()
        {
            var add = CompileAs<BinaryOp>(MakeSourceContext(), "Num.add");
            Assert.AreEqual(11f, add(3f, 8f));
        }

        [Test]
        public void CompileBinaryAsUnaryFails()
        {
            DoExpectingMessageCode(10, src =>
            {
                var fn = CompileAs<UnaryOp>(src,"Num.add");
                return fn != null;
            });
        }
        
        [Test]
        public void CompileUnaryAsBinaryFails()
        {
            DoExpectingMessageCode(10, src =>
            {
                var fn = CompileAs<BinaryOp>(src, "Num.sqr");
                return fn != null;
            });
        }

        [Test]
        public void ListWithStaticCount() => Assert.AreEqual(20f, CompileAs<Constant>(MakeSourceContext(), "List.repeat(4, 5).fold(0, Num.add)")());

        private static readonly int[] _dynamicListCounts = Enumerable.Repeat(1, 20).ToArray();

        [TestCaseSource(nameof(_dynamicListCounts))]
        public void ListWithDynamicCount(int count)
        {
            var dynamicList = CompileAs<UnaryOp>(MakeSourceContext(), "_(a:Num):Num = List.repeat(1, a).fold(0, Num.add)");
            Assert.AreEqual(count, dynamicList(count));
        }

        [Test]
        public void NoObjectBoundaryConverter()
        {
            DoExpectingMessageCode(12, src =>
            {
                var fn = CompileAs<InvalidDelegate>(src, "Num.sqr");
                return fn != null;
            });
        }

        [Test]
        public void TopLevelList()
        {
            var fn = CompileAs<IndexArray>(MakeSourceContext(), "_(list:List, idx:Num):Num = list.at(idx)");
            var thirdElement = fn(new List<float> {1f, 4f, 7f, -3f}, 3);
            Assert.AreEqual(7f, thirdElement);
        }

        [Test]
        public void IntermediateStructVectorAdd()
        {
            var fn = CompileAs<BinaryOp>(MakeSourceContext(extraSource: _compileSource), "_(a:Num, b:Num):Num = Vector3(a, a, a).add(Vector3(b, b, b)).x");
            var result = fn(5f, 10f);
            Assert.AreEqual(15f, result);
        }
        
        [Test]
        public void StructVectorAdd()
        {
            var fn = CompileAs<VectorOperation>(MakeSourceContext(extraSource: _compileSource), "Vector3.add");
            var result = fn(new Vector3(5f), new Vector3(10f));
            Assert.AreEqual(15f, result.X);
        }

        [Test]
        public void MakeCustomStructInstance()
        {
            var fn = CompileAs<CustomStructDelegate>(MakeSourceContext(extraSource: _compileSource), "_(f:Num, v3:Vector3):MyCustomElementStruct = MyCustomElementStruct(f, v3)");
            var result = fn(5f, new Vector3(10f));
            Assert.AreEqual(5f, result.floatField);
            Assert.AreEqual(10f, result.vector3Field.X);
        }

        private static string _customStructOperationSource = @"_(f:Num, v3:Vector3):MyCustomElementStruct
{
    fsqr = f.sqr;
    vadded = v3.add(Vector3(fsqr, fsqr, fsqr));
    return = MyCustomElementStruct(fsqr, vadded);
}";
        
        [Test]
        public void CustomStructOperations()
        {
            var fn = CompileAs<CustomStructDelegate>(MakeSourceContext(extraSource: _compileSource), _customStructOperationSource);
            var result = fn(5f, new Vector3(3f, 6f, -10f));
            Assert.AreEqual(25f, result.floatField);
            Assert.AreEqual(28f, result.vector3Field.X);
            Assert.AreEqual(31f, result.vector3Field.Y);
            Assert.AreEqual(15f, result.vector3Field.Z);
        }

        [Test]
        public void SourceArgumentFromSerializedArray()
        {
            var fn = CompileAs<CustomStructDelegateNoArgs>(MakeSourceContext(extraSource: _compileSource), _customStructOperationSource, out var args);
            Assert.AreEqual(4, args.Length);
            args[0] = 5f;
            args[1] = 3f;
            args[2] = 6f;
            args[3] = -10f;
            var result = fn();
            
            Assert.AreEqual(25f, result.floatField);
            Assert.AreEqual(28f, result.vector3Field.X);
            Assert.AreEqual(31f, result.vector3Field.Y);
            Assert.AreEqual(15f, result.vector3Field.Z);
        }

        private static readonly (float, float)[] _factorialArguments =
        {
            (0f, 1f),
            (1f, 1f),
            (2f, 2f),
            (3f, 6f),
            (4f, 24f),
            (5f, 120f),
            (6f, 720f),
            (7f, 5040f),
            (8f, 40320f),
            (9f, 362880f),
            (10f, 3628800f),
            (11f, 39916800f),
        };

        [TestCaseSource(nameof(_factorialArguments))]
        public void FactorialUsingFor((float fac, float result) f)
        {
            var fn = CompileAs<UnaryOp>(MakeSourceContext(), "_(a:Num):Num = for(Tuple(a, 1), _(tup):Bool = tup.varg0.gt(0), _(tup) = Tuple(tup.varg0.sub(1), tup.varg1.mul(tup.varg0))).varg1");
            Assert.AreEqual(f.result, fn(f.fac));
        }
    }
}