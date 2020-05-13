using Element.AST;
using Element.CLR;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class Uncurrying : FixtureBase
    {
        private delegate float AddSqr(float a, float b);

        [Test]
        public void UncurryAddMul()
        {
            var srcContext = MakeSourceContext();
            if (srcContext == null)
            {
                Assert.Fail();
                return;
            }
            var add = srcContext.EvaluateExpressionAs<IFunctionSignature>("Num.add", out _);
            var mul = srcContext.EvaluateExpressionAs<IFunctionSignature>("Num.sqr", out _);
            var uncurried = add.Uncurry(mul, srcContext);
            var (compiled, _) = srcContext.Compile<AddSqr>(uncurried);
            var result = compiled(5f, 10f);
            Assert.AreEqual(225f, result);
        }
        
        // TODO: cannot uncurry variadic function as param a
        // TODO: if return type of A is an Element type, check that B's first argument is the same type
    }
}