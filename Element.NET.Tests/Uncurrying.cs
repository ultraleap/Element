using Element.AST;
using Element.CLR;
using Element.NET.TestHelpers;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class Uncurrying : FixtureBase
    {
        private delegate float AddSqr(float a, float b);

        [Test]
        public void UncurryAddSqr()
        {
            var context = Context.CreateFromSourceContext(MakeSourceContext());
            context.EvaluateExpression("Num.add")
                   .Accumulate(() => context.EvaluateExpression("Num.sqr"))
                   .Bind(tuple =>
                    {
                        var (add, sqr) = tuple;
                        return add.Uncurry(sqr, context);
                    })
                   .Bind(uncurried => uncurried.Compile<AddSqr>(context))
                   .Switch((sqr, messages) =>
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