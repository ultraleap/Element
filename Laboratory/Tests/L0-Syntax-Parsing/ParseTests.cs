using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class ParseTests : HostFixture
    {
        public ParseTests(IHost host) : base(host) { }

        private const int _defaultFailingParseTestCode = 9;

        private static IEnumerable GenerateParseTestData()
        {
            var files = new DirectoryInfo("L0-Syntax-Parsing").GetFiles("*.ele", SearchOption.AllDirectories);
            foreach (var file in files)
            {
                var match = Regex.Match(Path.GetFileNameWithoutExtension(file.Name), @"(?<condition>(pass|fail){1})(?:-(?<value>\d+))?");
                if (!match.Success) continue;
                var expectedMessageCode = match.Groups["condition"].Value switch
                {
                    "pass" => (int?)null,
                    "fail" => int.TryParse(match.Groups["value"].Value, out var code) ? code : _defaultFailingParseTestCode
                };

                yield return new TestCaseData((FileInfo: file, ExpectMessageCode: expectedMessageCode)).SetName(file.FullName.Split("L0-Syntax-Parsing\\")[1]);
            }
        }

        [TestCaseSource(nameof(GenerateParseTestData))]
        public void ParseTest((FileInfo FileInfo, int? ExpectedMessageCode) info)
        {
            var (fileInfo, expectedMessageCode) = info;
            var success = _host.ParseFile(expectedMessageCode.HasValue
                    ? new CompilationInput(ExpectMessageCode(expectedMessageCode.Value), true)
                    : new CompilationInput(FailOnError, true),
                fileInfo);

            // When we have an error code but parsing succeeds, we have failed!
            if(expectedMessageCode.HasValue && success)
                Assert.Fail("Expected parsing to fail but no parse error was logged");
        }
    }
}