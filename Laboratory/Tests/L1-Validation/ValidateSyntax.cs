using System.Collections;
using System.IO;
using NUnit.Framework;

namespace Laboratory.Tests.L1.Validation
{
    internal class ValidateSyntax : SyntaxFixture
    {
        private static IEnumerable GenerateValidationTestData() => GenerateTestData("Validation", "L1-Validation", null);
        
        [TestCaseSource(nameof(GenerateValidationTestData))]
        public void Validation((FileInfo FileInfo, int? ExpectedMessageCode) info) => RunTest(info, false);
    }
}