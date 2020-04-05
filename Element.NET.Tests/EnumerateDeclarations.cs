using System;
using Element.AST;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class EnumerateDeclarations
    {
        protected static void FailOnError(CompilerMessage message)
        {
            if (message.MessageLevel >= MessageLevel.Error) Assert.Fail(message.ToString());
            else TestContext.WriteLine(message.ToString());
        }
        
        [Test]
        public void EnumerateAll()
        { 
            Assert.True(SourceContext.TryCreate(new CompilationInput(FailOnError), out var sourceContext));
            var results = sourceContext.GlobalScope.EnumerateDeclarations(_ => true);
            CollectionAssert.IsNotEmpty(results);
        }

        [
            TestCase("Num"),
            TestCase("Bool")
        ]
        public void EnumerateByName(string nameContains)
        {
            Assert.True(SourceContext.TryCreate(new CompilationInput(FailOnError), out var sourceContext));
            var results = sourceContext.GlobalScope.EnumerateDeclarations(v => v.Location.Contains(nameContains, StringComparison.OrdinalIgnoreCase));
            CollectionAssert.IsNotEmpty(results);
            // TODO: Actually check collection contents are correct
        }

        private static readonly IIntrinsic[] _types =
        {
            NumType.Instance,
            BoolType.Instance,
            ListType.Instance,
        };
        
        [TestCaseSource(nameof(_types))]
        public void EnumerateByReturnType(IIntrinsic type)
        {
            Assert.True(SourceContext.TryCreate(new CompilationInput(FailOnError), out var sourceContext));
            sourceContext.MakeCompilationContext(out var compilationContext);
            bool Filter(IValue v) => v is AST.IFunctionSignature fn && fn.Output.ResolveConstraint(compilationContext) == compilationContext.GetIntrinsicsDeclaration<DeclaredStruct>(type);
            var results = sourceContext.GlobalScope.EnumerateDeclarations(Filter);
            CollectionAssert.IsNotEmpty(results);
            // TODO: Actually check collection contents are correct
        }
        
        // Test case: don't recurse into function scopes! Returning intermediates would be dumb!
    }
}