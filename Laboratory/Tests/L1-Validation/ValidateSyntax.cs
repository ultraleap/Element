using System.Collections;
using System.IO;
using Element;
using NUnit.Framework;
using ResultNET;

namespace Laboratory.Tests.L1.Validation
{
    internal class ValidateSyntax : SyntaxFixture
    {
        private static IEnumerable GenerateValidationTestData() => GenerateTestData("Validation", "L1-Validation", ElementMessage.MessagePrefix, null);

        [TestCaseSource(nameof(GenerateValidationTestData))]
        public void Validation((FileInfo fileInfo, MessageInfo? messageInfo) info) => SyntaxTest(info, false, false);
    }
}