using System;
using System.Collections;
using System.Collections.Generic;
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
                var expectedResult = match.Groups["condition"].Value switch
                {
                    "pass" => true,
                    "fail" => false,
                    _ => throw new ArgumentOutOfRangeException()
                };

                var expectedMessageCode = expectedResult
                    ? (int?) null
                    : int.TryParse(match.Groups["value"].Value, out var code)
                        ? code
                        : defaultExpectedErrorCode ??
                          throw new ArgumentNullException(nameof(defaultExpectedErrorCode),
                              $"Error code must be specified explicitly for {testKind} tests");
  
                yield return new TestCaseData((file, expectedMessageCode)).SetName($"{testKind}{file.FullName.Split(directory)[1]}");
            }
        }
        
        public void SyntaxTest((FileInfo, int?) info, bool skipValidation)
        {
            var (fileInfo, messageCode) = info;
            var expectingError = messageCode.HasValue;
            
            var messages = new List<CompilerMessage>();
            var compilationInput = new CompilationInput(CacheMessage(messages))
            {
                ExcludePrelude = true,
                ExtraSourceFiles = new[]{fileInfo},
                SkipValidation = skipValidation,
                NoParseTrace = expectingError
            };
            
            var success = Host.Parse(compilationInput);
            if(expectingError)
                ExpectingError(messages, success, messageCode.Value);
            else
                ExpectingSuccess(messages, success);
        }
    }
}