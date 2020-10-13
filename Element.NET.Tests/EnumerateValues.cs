using System;
using Element.AST;
using Element.NET.TestHelpers;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class EnumerateValues : FixtureBase
    {
        [Test]
        public void EnumerateAll()
        {
            var sourceContext = MakeSourceContext();
            var results = sourceContext.GlobalScope.EnumerateValues(new Context(sourceContext));
            Assert.That(results.IsSuccess);
            CollectionAssert.IsNotEmpty(results.ResultOr(default));
        }

        [
            TestCase("Num"),
            TestCase("Bool")
        ]
        public void EnumerateByName(string nameContains)
        {
            var sourceContext = MakeSourceContext();
            var results = sourceContext.GlobalScope.EnumerateValues(new Context(sourceContext), d => d.Identifier.String.Contains(nameContains, StringComparison.OrdinalIgnoreCase));
            Assert.That(results.IsSuccess);
            CollectionAssert.IsNotEmpty(results.ResultOr(default));
            // TODO: Actually check collection contents are correct
        }

        private static readonly IIntrinsicImplementation[] _intrinsics =
        {
            NumStruct.Instance,
            BoolStruct.Instance,
            ListStruct.Instance,
        };
        
        [TestCaseSource(nameof(_intrinsics))]
        public void EnumerateByReturnType(IIntrinsicImplementation intrinsic)
        {
            var sourceContext = MakeSourceContext();
            bool Filter(ValueWithLocation v) => v.Value.ReturnConstraint.IsSpecificIntrinsic(intrinsic);
            var results = sourceContext.GlobalScope.EnumerateValues(new Context(sourceContext), resolvedValueFilter: Filter);
            Assert.That(results.IsSuccess);
            CollectionAssert.IsNotEmpty(results.ResultOr(default));
            // TODO: Actually check collection contents are correct
        }
        
        // Test case: don't recurse into function scopes! Returning intermediates would be bad!
    }
}