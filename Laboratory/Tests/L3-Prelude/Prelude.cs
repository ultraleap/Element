using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class Prelude : PreludeFixture
    {
        [Test]
        // Evaluates an arbitrary expression since creating the prelude will be parsed implicitly when not excluded
        public void Parse() => Assert.True(Host.Parse(NonValidatedCompilationInput));
        
        [Test]
        // Evaluates an arbitrary expression since creating the prelude will be parsed implicitly when not excluded
        public void Validate() =>  Assert.True(Host.Parse(ValidatedCompilationInput));
    }
}