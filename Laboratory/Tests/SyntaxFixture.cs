using System;
using System.Collections;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal abstract class SyntaxFixture : HostFixture
    {
        protected const int DefaultFailingParseTestCode = 9;
        
        protected static IEnumerable GenerateTestData(string testKind, string directory, int? defaultExpectedErrorCode)
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
                                    throw new ArgumentNullException(nameof(defaultExpectedErrorCode), $"Error code must be specified explicitly for {testKind} tests"),
                    _ => throw new ArgumentOutOfRangeException()
                };

                yield return new TestCaseData((FileInfo: file, ExpectMessageCode: expectedMessageCode)).SetName($"{testKind}{file.FullName.Split(directory)[1]}");
            }
        }
        
        protected void RunTest((FileInfo FileInfo, int? ExpectedMessageCode) info, bool skipValidation)
        {
            var (fileInfo, expectedMessageCode) = info;
            var errors = new System.Collections.Generic.List<CompilerMessage>();
            var compilationInput = new CompilationInput(expectedMessageCode.HasValue ? ExpectMessageCode(expectedMessageCode.Value, errors) : FailOnError)
            {
                ExcludePrelude = true,
                ExtraSourceFiles = new[]{fileInfo},
                SkipValidation = skipValidation
            };
            
            var success = Host.Parse(compilationInput);

            if (!expectedMessageCode.HasValue) return; //Handled by FailOnError

            if (success)
            {
                Assert.Fail("Expected error ELE{0} '{1} but succeeded",
                    expectedMessageCode.Value, CompilerMessage.GetMessageName(expectedMessageCode.Value));
            }
            else
            {
                if (errors.Count >= 0)
                {
                    Assert.Fail("Expected error ELE{0} '{1}' but got following error codes instead: {2}",
                        expectedMessageCode.Value, CompilerMessage.GetMessageName(expectedMessageCode.Value),
                        string.Join(",", errors.Select(err => err.MessageCode)));
                }
                else
                {
                    Assert.Fail("Expected error ELE{0} '{1}' but no error was logged", expectedMessageCode.Value,
                        CompilerMessage.GetMessageName(expectedMessageCode.Value));
                }
            }
        }
    }
}