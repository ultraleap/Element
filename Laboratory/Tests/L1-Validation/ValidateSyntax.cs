using System.Collections;
using System.IO;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L1.Validation
{
    internal class ValidateSyntax : SyntaxFixture
    {
        private static IEnumerable GenerateValidationTestData() => GenerateTestData("Validation", "L1-Validation", null);

        [TestCaseSource(nameof(GenerateValidationTestData))]
        public void Validation((FileInfo fileInfo, MessageCode? messageCode) info) => SyntaxTest(info, false);
    }
}