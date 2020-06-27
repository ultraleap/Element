using System;
using Element.AST;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class EnumerateValues : FixtureBase
    {
        [Test]
        public void EnumerateAll()
        {
            var sourceContext = MakeSourceContext();
            var results = sourceContext.GlobalScope.EnumerateRecursively(_ => true);
            CollectionAssert.IsNotEmpty(results);
        }

        [
            TestCase("Num"),
            TestCase("Bool")
        ]
        public void EnumerateByName(string nameContains)
        {
            var sourceContext = MakeSourceContext();
            var results = sourceContext.GlobalScope.EnumerateDeclarationsRecursively(d => d.Location.Contains(nameContains, StringComparison.OrdinalIgnoreCase));
            CollectionAssert.IsNotEmpty(results);
            // TODO: Actually check collection contents are correct
        }

        private static readonly IntrinsicType[] _types =
        {
            NumType.Instance,
            BoolType.Instance,
            ListType.Instance, 
            TupleType.Instance,
        };
        
        [TestCaseSource(nameof(_types))]
        public void EnumerateByReturnType(IntrinsicType type)
        {
            var sourceContext = MakeSourceContext();
            var compilationContext = new CompilationContext(sourceContext);
            bool Filter(Declaration d) => d is IFunctionSignature fn
                                          && fn.Output.ResolveConstraint(compilationContext)
                                               .Match((t, _) => t.IsIntrinsicType(type),
                                                      _ => false);
            var results = sourceContext.GlobalScope.EnumerateDeclarationsRecursively(Filter);
            CollectionAssert.IsNotEmpty(results);
            // TODO: Actually check collection contents are correct
        }
        
        // Test case: don't recurse into function scopes! Returning intermediates would be bad!
    }
}