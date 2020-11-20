using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using Element.CLR;
using Element.NET.TestHelpers;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class Compile : FixtureBase
    {
        private const string _compileSource = @"struct Vector3(x:Num, y:Num, z:Num)
{
    add(a:Vector3, b:Vector3) = Vector3(a.x.add(b.x), a.y.add(b.y), a.z.add(b.z))
}
struct MyCustomElementStruct(floatField:Num, vector3Field:Vector3)
struct CustomNestedStruct(structField:MyCustomElementStruct, floatField:Num, vector3Field:Vector3)
";

        [ElementBoundaryStruct("MyCustomElementStruct")]
        private struct MyCustomElementStruct
        {
            public float floatField;
            public Vector3 vector3Field;
        }
        
        [ElementBoundaryStruct("CustomNestedStruct")]
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

        private static Result<(TDelegate Delegate, float[] ArgumentArray)> CompileAndSourceArguments<TDelegate>(SourceContext context, string expression)
            where TDelegate : Delegate =>
            Context.CreateFromSourceContext(context).ToDefaultBoundaryContext()
                   .Bind(boundaryContext => boundaryContext.EvaluateExpression(expression).Map(function => (function, boundaryContext)))
                   .Bind(tuple => tuple.function.SourceArgumentsFromSerializedArray(tuple.boundaryContext).Map(valueTuple => (valueTuple, tuple.boundaryContext)))
                   .Bind(tuple =>
                   {
                       var ((capturingValue, captureArray), boundaryContext) = tuple;
                       return capturingValue.Compile<TDelegate>(boundaryContext).Map(compiled => (compiled, captureArray));
                   });
        
        private static void CompileWithSourcedArgsAndCheck<TDelegate>(SourceContext sourceContext, string expression, Action<TDelegate, float[]> checkFunc)
            where TDelegate : Delegate =>
            CompileAndSourceArguments<TDelegate>(sourceContext, expression)
                .Switch((t, messages) =>
                {
                    LogMessages(messages);
                    checkFunc(t.Delegate, t.ArgumentArray);
                }, messages => ExpectingSuccess(messages, false));
        
        private static Result<TDelegate> CompileDelegate<TDelegate>(SourceContext context, string expression)
            where TDelegate : Delegate =>
            Context.CreateFromSourceContext(context).ToDefaultBoundaryContext()
                   .Bind(boundaryContext => boundaryContext.EvaluateExpression(expression).Map(function => (function, boundaryContext)))
                   .Bind(tuple => tuple.function.Compile<TDelegate>(tuple.boundaryContext));
        
        private static void CompileAndCheck<TDelegate>(SourceContext sourceContext, string expression, Action<TDelegate> checkFunc)
            where TDelegate : Delegate =>
            CompileDelegate<TDelegate>(sourceContext, expression)
                .Switch((fn, messages) =>
                {
                    LogMessages(messages);
                    checkFunc(fn);
                }, messages => ExpectingSuccess(messages, false));

        private static void CompileAndCheck<TDelegate>(string expression, Action<TDelegate> checkFunc)
            where TDelegate : Delegate =>
            CompileAndCheck(MakeSourceContext(), expression, checkFunc);

        [Test]
        public void ConstantPi() => CompileAndCheck<Constant>("Num.pi", piFn => Assert.AreEqual(3.14159265359f, piFn()));

        [Test]
        public void BinaryAdd() => CompileAndCheck<BinaryOp>("Num.add", addFn => Assert.AreEqual(11f, addFn(3f, 8f)));

        [Test]
        public void CompileBinaryAsUnaryFails()
        {
            var fn = CompileDelegate<UnaryOp>(MakeSourceContext(), "Num.add");
            ExpectingElementError(fn.Messages, fn.IsSuccess, EleMessageCode.InvalidBoundaryFunction);
        }
        
        [Test]
        public void CompileUnaryAsBinaryFails()
        {
            var fn = CompileDelegate<BinaryOp>(MakeSourceContext(), "Num.sqr");
            ExpectingElementError(fn.Messages, fn.IsSuccess, EleMessageCode.InvalidBoundaryFunction);
        }

        [Test]
        public void ListWithStaticCount() => CompileAndCheck<Constant>("List.repeat(4, 5).fold(0, Num.add)", fn => Assert.AreEqual(20f, fn()));

        private static readonly int[] _dynamicListCounts = Enumerable.Repeat(1, 20).ToArray();

        [TestCaseSource(nameof(_dynamicListCounts))]
        public void ListWithDynamicCount(int count) => CompileAndCheck<UnaryOp>("_(a:Num):Num = List.repeat(1, a).fold(0, Num.add)", fn => Assert.AreEqual(count, fn(count)));

        [Test]
        public void NoObjectBoundaryConverter()
        {
            var fn = CompileDelegate<InvalidDelegate>(MakeSourceContext(), "Num.sqr");
            ExpectingElementError(fn.Messages, fn.IsSuccess, EleMessageCode.MissingBoundaryConverter);
        }

        [Test]
        public void TopLevelList() =>
            CompileAndCheck<IndexArray>("_(list:List, idx:Num):Num = list.at(idx)", fn =>
            {
                var thirdElement = fn(new List<float> {1f, 4f, 7f, -3f}, 3);
                Assert.AreEqual(7f, thirdElement);
            });

        [Test]
        public void IntermediateStructVectorAdd() =>
            CompileAndCheck<BinaryOp>(MakeSourceContext(extraSource: _compileSource),
                                      "_(a:Num, b:Num):Num = Vector3(a, a, a).add(Vector3(b, b, b)).x",
                                      fn => Assert.AreEqual(15f, fn(5f, 10f)));

        [Test]
        public void StructVectorAdd() =>
            CompileAndCheck<VectorOperation>(MakeSourceContext(extraSource: _compileSource),
                                             "Vector3.add",
                                             fn =>
                                             {
                                                 var result = fn(new Vector3(5f), new Vector3(10f));
                                                 Assert.AreEqual(15f, result.X);
                                             });

        [Test]
        public void MakeCustomStructInstance() => 
            CompileAndCheck<CustomStructDelegate>(MakeSourceContext(extraSource: _compileSource),
                                                  "_(f:Num, v3:Vector3):MyCustomElementStruct = MyCustomElementStruct(f, v3)",
                                                  fn =>
                                                  {
                                                      var result = fn(5f, new Vector3(10f));
                                                      Assert.AreEqual(5f, result.floatField);
                                                      Assert.AreEqual(10f, result.vector3Field.X);
                                                  });

        private static string _customStructOperationSource = @"
_(f:Num, v3:Vector3):MyCustomElementStruct
{
    fsqr = f.sqr
    vadded = v3.add(Vector3(fsqr, fsqr, fsqr))
    return = MyCustomElementStruct(fsqr, vadded)
}
";

        [Test]
        public void CustomStructOperations() =>
            CompileAndCheck<CustomStructDelegate>(MakeSourceContext(extraSource: _compileSource),
                                                  _customStructOperationSource,
                                                  fn =>
                                                  {
                                                      var result = fn(5f, new Vector3(3f, 6f, -10f));
                                                      Assert.AreEqual(25f, result.floatField);
                                                      Assert.AreEqual(28f, result.vector3Field.X);
                                                      Assert.AreEqual(31f, result.vector3Field.Y);
                                                      Assert.AreEqual(15f, result.vector3Field.Z);
                                                  });

        [Test]
        public void SourceArgumentFromSerializedArray() =>
            CompileWithSourcedArgsAndCheck<CustomStructDelegateNoArgs>(MakeSourceContext(extraSource: _compileSource),
                                                                       _customStructOperationSource,
                                                                       (fn, args) =>
                                                                       {
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
                                                                       });

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
        public void FactorialUsingFor((float fac, float result) f) =>
            CompileAndCheck<UnaryOp>("_(a:Num):Num = for({n = a, i = 1}, _(tup):Bool = tup.n.gt(0), _(tup) = {n = tup.n.sub(1), i = tup.i.mul(tup.n)}).i",
                                     fn => Assert.AreEqual(f.result, fn(f.fac)));
    }
}