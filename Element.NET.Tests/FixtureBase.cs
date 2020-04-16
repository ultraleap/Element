using NUnit.Framework;

namespace Element.NET.Tests
{
    public abstract class FixtureBase
    {
        protected static void FailOnError(CompilerMessage message)
        {
            if (message.MessageLevel >= MessageLevel.Error) Assert.Fail(message.ToString());
            else TestContext.WriteLine(message.ToString());
        }

        protected SourceContext? MakeSourceContext(CompilationInput compilationInput = default) =>
            SourceContext.TryCreate(compilationInput ?? new CompilationInput(FailOnError), out var sourceContext) ? sourceContext : null;
    }
}