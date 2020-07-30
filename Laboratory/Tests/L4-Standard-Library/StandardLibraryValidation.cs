using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Validation : StandardLibraryFixture
    {
        [Test]
        // Evaluates an arbitrary expression since creating the prelude will be parsed implicitly when not excluded
        public void Parse()
        {
            var result = Host.Parse(NonValidatedCompilationInput);
            ExpectingSuccess(result.Messages, result.IsSuccess);
        }

        [Test]
        // Evaluates an arbitrary expression since creating the prelude will be parsed implicitly when not excluded
        public void Validate()
        {
            var result = Host.Parse(ValidatedCompilationInput);
            ExpectingSuccess(result.Messages, result.IsSuccess);
        }
    }
}