using Element.AST;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class EnumerateValuesTests
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
            var results = sourceContext.GlobalScope.EnumerateValues(_ => true);
            CollectionAssert.IsNotEmpty(results);
        }

        [
            TestCase("Num"),
            TestCase("Bool")
        ]
        public void EnumerateByName(string nameContains)
        {
            Assert.True(SourceContext.TryCreate(new CompilationInput(FailOnError), out var sourceContext));
            var results = sourceContext.GlobalScope.EnumerateValues(_ => true);
            CollectionAssert.IsNotEmpty(results);
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
            var compilationContext = sourceContext.MakeCompilationContext();
            bool Filter(IValue v) => v is AST.IFunction fn && fn.Output.ResolveConstraint(compilationContext) == type.GetDeclaration<DeclaredStruct>(compilationContext);
            var results = sourceContext.GlobalScope.EnumerateValues(Filter);
            CollectionAssert.IsNotEmpty(results);
        }
    }
}