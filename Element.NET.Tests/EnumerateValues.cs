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
            var results = sourceContext.GlobalScope.EnumerateValues<IValue>(_ => true);
            CollectionAssert.IsNotEmpty(results);
        }

        [
            TestCase("Num"),
            TestCase("Bool")
        ]
        public void EnumerateByName(string nameContains)
        {
            var sourceContext = MakeSourceContext();
            var results = sourceContext.GlobalScope.EnumerateValues<Declaration>(v => v.Location.Contains(nameContains, StringComparison.OrdinalIgnoreCase));
            CollectionAssert.IsNotEmpty(results);
            // TODO: Actually check collection contents are correct
        }

        private static readonly IIntrinsic[] _types =
        {
            NumType.Instance,
            BoolType.Instance,
            ListType.Instance,
            TupleType.Instance,
        };
        
        [TestCaseSource(nameof(_types))]
        public void EnumerateByReturnType(IIntrinsic type)
        {
            var sourceContext = MakeSourceContext();
            sourceContext.MakeCompilationContext(out var compilationContext);
            bool Filter(IValue v) => v is IFunctionSignature fn && fn.Output.ResolveConstraint(sourceContext.GlobalScope, compilationContext) == compilationContext.GetIntrinsicsDeclaration<DeclaredStruct>(type);
            var results = sourceContext.GlobalScope.EnumerateValues<Declaration>(Filter);
            CollectionAssert.IsNotEmpty(results);
            // TODO: Actually check collection contents are correct
        }
        
        // Test case: don't recurse into function scopes! Returning intermediates would be dumb!
    }
}