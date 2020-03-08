using Element;

namespace Laboratory
{
    internal abstract class PreludeFixture : HostFixture
    {
        protected PreludeFixture(IHost host) : base(host) { }

        protected CompilationInput CompilationInput => new CompilationInput(FailOnError);
    }
}