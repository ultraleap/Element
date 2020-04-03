using NUnit.Framework;

namespace Laboratory.Tests
{
    class StandardLibrary : StandardLibraryFixture
    {
        [Test]
        // Evaluates an arbitrary expression since creating the prelude will be parsed implicitly when not excluded
        public void Parse() => Assert.True(Host.Parse(NonValidatedCompilationInput));
        
        [Test]
        // Evaluates an arbitrary expression since creating the prelude will be parsed implicitly when not excluded
        public void Validate() =>  Assert.True(Host.Parse(ValidatedCompilationInput));
    }
}