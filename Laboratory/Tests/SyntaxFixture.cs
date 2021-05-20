using System;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using Element;
using Element.NET.TestHelpers;
using NUnit.Framework;
using ResultNET;

namespace Laboratory.Tests
{
    internal abstract class SyntaxFixture : HostFixture
    {
        protected static readonly MessageInfo DefaultFailingParseTestMessageInfo = ElementMessage.ParseError;
        
        // TODO: Get MessageTypePrefix from the test filename instead of having to pass in
        protected static IEnumerable GenerateTestData(string testKind, string directory, string messageTypePrefix, MessageInfo? defaultExpectedError)
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
                    ? null
                    : int.TryParse(match.Groups["value"].Value, out var code)
                        ? MessageInfo.GetByPrefixAndCode(messageTypePrefix, code)
                        : defaultExpectedError
                          ?? throw new ArgumentNullException(nameof(defaultExpectedError), $"Error code must be specified explicitly for {testKind} tests");
  
                yield return new TestCaseData((file, expectedMessageCode)).SetName($"{testKind}{file.FullName.Split(directory)[1]}");
            }
        }
        
        public void SyntaxTest((FileInfo ParseTestFile, MessageInfo? ExpectedMessageCode) info, bool skipValidation, bool showParseTraceForErrors)
        {
            var (fileInfo, messageInfo) = info;
            var expectingError = messageInfo != null;

            var compilationInput = new CompilerInput(TestPackageRegistry,
                                                     null,
                                                     Array.Empty<PackageSpecifier>(),
                                                     new[] {fileInfo},
                                                     new CompilerOptions(default, default, skipValidation, expectingError && !showParseTraceForErrors));

            var result = Host.Parse(compilationInput);
            if (expectingError)
                ExpectingError(result.Messages, result.IsSuccess, messageInfo!);
            else
                ExpectingSuccess(result.Messages, result.IsSuccess);
        }
    }
}