using System;
using System.Collections;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using Element;
using Element.AST;
using Lexico;
using NUnit.Framework;
using Expression = Element.AST.Expression;
using Type = System.Type;

namespace Laboratory.Tests
{
    internal class ParseAndValidationTests : HostFixture
    {
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
                                    throw new ArgumentNullException(nameof(defaultExpectedErrorCode), $"Error code must be specified explicitly for {testKind} tests"),
                    _ => throw new ArgumentOutOfRangeException()
                };

                yield return new TestCaseData((FileInfo: file, ExpectMessageCode: expectedMessageCode)).SetName($"{testKind}{file.FullName.Split(directory)[1]}");
            }
        }
        
        private void RunTest((FileInfo FileInfo, int? ExpectedMessageCode) info, bool skipValidation)
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

            if (expectedMessageCode.HasValue && success)
            {
                if (errors.Count > 0)
                {
                    Assert.Fail("Expected error ELE{0} '{1}' but got following error codes instead: {2}",
                        expectedMessageCode.Value, CompilerMessage.GetMessageName(expectedMessageCode.Value),
                        string.Join(",", errors.Select(err => err.MessageCode)));
                } else {
                    Assert.Fail("Expected error ELE{0} '{1}' but no error was logged", expectedMessageCode.Value,
                        CompilerMessage.GetMessageName(expectedMessageCode.Value));
                }
            }
        }

        private static IEnumerable GenerateParseTestData() => GenerateTestData("Parse", "L0-Parsing", _defaultFailingParseTestCode);
        private static IEnumerable GenerateValidationTestData() => GenerateTestData("Validation", "L1-Validation", null);
        
        [TestCaseSource(nameof(GenerateParseTestData))]
        public void ParseTest((FileInfo FileInfo, int? ExpectedMessageCode) info) => RunTest(info, true);
        
        [TestCaseSource(nameof(GenerateValidationTestData))]
        public void ValidationTest((FileInfo FileInfo, int? ExpectedMessageCode) info) => RunTest(info, false);

        private static IEnumerable PartialSyntaxTestData()
        {
            var data = new[]
            {
                (nameof(Identifier), "a", typeof(Identifier)),
                (nameof(Unidentifier), "_", typeof(Unidentifier)),
                (nameof(Literal), "5", typeof(Literal)),
                (nameof(Terminal), ";", typeof(Terminal)),
                (nameof(ExpressionBody), "= 5", typeof(ExpressionBody)),
                (nameof(Binding), "= 5;", typeof(Binding)),
                (nameof(TypeAnnotation), ":Foo.Bar", typeof(TypeAnnotation)),
                (nameof(Expression), "a.b(c).d.e(5)", typeof(Expression)),
                (nameof(CallExpression), "(5, a(10, c))", typeof(CallExpression)),
                (nameof(IndexingExpression), ".identifier", typeof(IndexingExpression)),
                (nameof(Lambda), "_(_a) = 5", typeof(Lambda)),
                ($"{nameof(ExtrinsicFunction)}-ExpressionBody", "a(a):Num = 5;", typeof(ExtrinsicFunction)),
                ($"{nameof(ExtrinsicFunction)}-ScopeBody", "a(a):Num {return = 5;}", typeof(ExtrinsicFunction)),
                ($"{nameof(IntrinsicFunction)}", "intrinsic a(a):Num;", typeof(IntrinsicFunction)),
                (nameof(ExtrinsicConstraint), "constraint a(a):Num;", typeof(ExtrinsicConstraint)),
                ($"{nameof(IntrinsicConstraint)}", "intrinsic constraint a;", typeof(IntrinsicConstraint)),
                (nameof(ExtrinsicStruct), "struct a(a);", typeof(ExtrinsicStruct)),
                ($"{nameof(ExtrinsicStruct)}-WithScope", "struct a(a) {}", typeof(ExtrinsicStruct)),
                ($"{nameof(IntrinsicStruct)}", "intrinsic struct a(a);", typeof(IntrinsicStruct)),
                (nameof(Port), "a:foo", typeof(Port)),
                (nameof(PortList), "(a:foo, b:bar)", typeof(PortList)),
                (nameof(DeclaredNamespace), "namespace foo {}", typeof(DeclaredNamespace)),
            };
            foreach (var item in data)
            {
                yield return new TestCaseData((item.Item2, item.Item3)).SetName($"Syntax-{item.Item1}");
            }
        }

        // TODO: Force consuming the whole text in all of these parsers
        // TODO: Make this test use host so that it can test process hosts!
        [TestCaseSource(nameof(PartialSyntaxTestData))]
        public void ParsePartialSyntaxItems((string text, Type syntaxItem) info)
        {
            if(!(Host is AtomicHost)) Assert.Inconclusive("Test only implemented for self host");
            Assert.That(Lexico.Lexico.TryParse(info.text, info.syntaxItem, out _, new ConsoleTrace()), Is.True);
        }
    }
}