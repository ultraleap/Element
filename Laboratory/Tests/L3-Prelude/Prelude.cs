using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class Prelude : PreludeFixture
    {
        public Prelude(IHost host) : base(host) { }

        [Test]
        // Evaluates an arbitrary expression since creating the prelude will be parsed implicitly when not excluded
        public void Parse() => Host.Evaluate(NonValidatedCompilationInput, "3.14");
        
        [Test]
        // Evaluates an arbitrary expression since creating the prelude will be parsed implicitly when not excluded
        public void Validate() => Host.Evaluate(ValidatedCompilationInput, "3.14");
    }
}