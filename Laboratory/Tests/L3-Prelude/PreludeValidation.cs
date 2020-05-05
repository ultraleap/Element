using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
    internal class Validation : PreludeFixture
    {
        [Test]
        // Only parses, does not validate
        public void Parse() => Assert.True(Host.Parse(new CompilationInput(FailOnError) {SkipValidation = true}));
        
        [Test]
        public void Validate() =>  Assert.True(Host.Parse(CompilationInput));
    }
}