using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
    internal class Validation : PreludeFixture
    {
        [Test]
        // Only parses, does not validate
        public void Parse()
        {
            var result = Host.Parse(NonValidatedCompilationInput);
            ExpectingSuccess(result.Messages, result.IsSuccess);
        }

        [Test]
        public void Validate()
        {
            var result = Host.Parse(ValidatedCompilationInput);
            ExpectingSuccess(result.Messages, result.IsSuccess);
        }
    }
}