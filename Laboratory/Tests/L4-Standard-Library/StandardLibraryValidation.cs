using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Validation : StandardLibraryFixture
    {
        [Test]
        // Evaluates an arbitrary expression since creating the prelude will be parsed implicitly when not excluded
        public void Parse()
        {
            var result = Host.Parse(NonValidatedCompilerInput);
            ExpectingSuccess(result.Messages, result.IsSuccess);
        }

        [Test]
        // Evaluates an arbitrary expression since creating the prelude will be parsed implicitly when not excluded
        public void Validate()
        {
            var result = Host.Parse(ValidatedCompilerInput);
            ExpectingSuccess(result.Messages, result.IsSuccess);
        }
    }
}