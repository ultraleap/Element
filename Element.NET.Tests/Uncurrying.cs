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
            srcContext.EvaluateExpression("Num.add").Cast<IFunctionSignature>(srcContext)
                                   .Accumulate(() => srcContext.EvaluateExpression("Num.sqr").Cast<IFunctionSignature>(srcContext))
                                   .Bind(tuple =>
                                   {
                                       var (add, sqr) = tuple;
                                       return add.Uncurry(sqr, srcContext);
                                   })
                                   .Bind(uncurried => uncurried.Compile<AddSqr>(new CompilationContext(srcContext)))
                                   .Match((sqr, messages) =>
                                   {
                                       LogMessages(messages);
                                       var result = sqr(5f, 10f);
                                       Assert.AreEqual(225f, result);
                                   }, messages => ExpectingSuccess(messages, false));
        }
        
        // TODO: cannot uncurry variadic function as param a
        // TODO: if return type of A is an Element type, check that B's first argument is the same type
    }
}