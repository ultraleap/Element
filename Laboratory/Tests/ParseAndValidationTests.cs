using System;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class ParseAndValidationTests : HostFixture
    {
        public ParseAndValidationTests(IHost host) : base(host) { }

        private const int _defaultFailingParseTestCode = 9;

        private static IEnumerable GenerateTestData(string testKind, string directory, int? defaultExpectedErrorCode)
        {
            var files = new DirectoryInfo(directory).GetFiles("*.ele", SearchOption.AllDirectories);
            foreach (var file in files)
            {
                var match = Regex.Match(Path.GetFileNameWithoutExtension(file.Name), @"(?<condition>(pass|fail){1})(?:-(?<value>\d+))?");
                if (!match.Success) continue;
                var expectedMessageCode = match.Groups["condition"].Value switch
                {
                    "pass" => (int?) null,
                    "fail" => int.TryParse(match.Groups["value"].Value, out var code)
                                  ? code
                                  : defaultExpectedErrorCode ??
                                    throw new ArgumentNullException(nameof(defaultExpectedErrorCode), $"Error code must be specified explicitly for {testKind} tests")
                };

                yield return new TestCaseData((FileInfo: file, ExpectMessageCode: expectedMessageCode)).SetName($"{testKind}{file.FullName.Split(directory)[1]}");
            }
        }
        
        private void RunTest((FileInfo FileInfo, int? ExpectedMessageCode) info, bool skipValidation)
        {
            var (fileInfo, expectedMessageCode) = info;
            var compilationInput = new CompilationInput(expectedMessageCode.HasValue ? ExpectMessageCode(expectedMessageCode.Value, FailOnError) : FailOnError)
            {
                ExcludePrelude = true,
                SkipValidation = skipValidation
            };
            var success = _host.ParseFile(compilationInput, fileInfo);

            if (expectedMessageCode.HasValue && success)
                Assert.Fail($"Expected error ELE{expectedMessageCode.Value} '{CompilerMessage.GetMessageName(expectedMessageCode.Value)}' but no error was logged");
        }

        private static IEnumerable GenerateParseTestData() => GenerateTestData("Parse", "L0-Parsing", _defaultFailingParseTestCode);
        private static IEnumerable GenerateValidationTestData() => GenerateTestData("Validation", "L1-Validation", null);
        
        [TestCaseSource(nameof(GenerateParseTestData))]
        public void ParseTest((FileInfo FileInfo, int? ExpectedMessageCode) info) => RunTest(info, true);
        
        [TestCaseSource(nameof(GenerateValidationTestData))]
        public void ValidationTest((FileInfo FileInfo, int? ExpectedMessageCode) info) => RunTest(info, false);
    }
}