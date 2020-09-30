using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
    internal class Validation : PreludeFixture
    {
        [Test]
        // Only parses, does not validate
        public void Parse()
        {
            var result = Host.Parse(NonValidatedCompilerInput);
            ExpectingSuccess(result.Messages, result.IsSuccess);
        }

        [Test]
        public void Validate()
        {
            var result = Host.Parse(ValidatedCompilerInput);
            ExpectingSuccess(result.Messages, result.IsSuccess);
        }
    }
}