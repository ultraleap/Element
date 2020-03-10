using Element;

namespace Laboratory.Tests
{
    internal abstract class PreludeFixture : HostFixture
    {
        protected PreludeFixture(IHost host) : base(host) { }

        protected CompilationInput ValidatedCompilationInput { get; } = new CompilationInput(FailOnError);
        protected CompilationInput NonValidatedCompilationInput { get; } = new CompilationInput(FailOnError) {SkipValidation = true};
    }
}