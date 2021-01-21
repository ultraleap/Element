using System;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using Element;
using Element.NET.TestHelpers;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal abstract class SyntaxFixture : HostFixture
    {
        protected static readonly EleMessageCode DefaultFailingParseTestCode = EleMessageCode.ParseError;
        
        protected static IEnumerable GenerateTestData(string testKind, string directory, EleMessageCode? defaultExpectedErrorCode)
        {
            var files = new DirectoryInfo(directory).GetFiles("*.ele", SearchOption.AllDirectories);
            foreach (var file in files)
            {
                var match = Regex.Match(Path.GetFileNameWithoutExtension(file.Name), @"(?<condition>(pass|fail){1})(?:-(?<value>\d+))?");
                if (!match.Success) continue;
                var expectedResult = match.Groups["condition"].Value switch
                {
                    "pass" => true,
                    "fail" => false,
                    _ => throw new ArgumentOutOfRangeException()
                };

                var expectedMessageCode = expectedResult
                    ? (EleMessageCode?) null
                    : int.TryParse(match.Groups["value"].Value, out var code)
                        ? (EleMessageCode)code
                        : defaultExpectedErrorCode
                          ?? throw new ArgumentNullException(nameof(defaultExpectedErrorCode), $"Error code must be specified explicitly for {testKind} tests");
  
                yield return new TestCaseData((file, expectedMessageCode)).SetName($"{testKind}{file.FullName.Split(directory)[1]}");
            }
        }
        
        public void SyntaxTest((FileInfo ParseTestFile, EleMessageCode? ExpectedMessageCode) info, bool skipValidation, bool showParseTraceForErrors)
        {
            var (fileInfo, messageCode) = info;
            var expectingError = messageCode.HasValue;

            var compilationInput = new CompilerInput(TestPackageRegistry,
                                                     null,
                                                     Array.Empty<PackageSpecifier>(),
                                                     new[] {fileInfo},
                                                     new CompilerOptions(default, default, skipValidation, expectingError && !showParseTraceForErrors));

            var result = Host.Parse(compilationInput);
            if (expectingError)
                ExpectingError(result.Messages, result.IsSuccess, MessageExtensions.TypeString, (int)messageCode!.Value);
            else
                ExpectingSuccess(result.Messages, result.IsSuccess);
        }
    }
}