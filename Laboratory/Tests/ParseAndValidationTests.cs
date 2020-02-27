using System;
using System.Collections;
using System.IO;
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

        private static IEnumerable PartialSyntaxTestData()
        {
            var data = new[]
            {
                (nameof(Identifier), "a", typeof(Identifier)),
                (nameof(Literal), "5", typeof(Literal)),
                (nameof(Terminal), ";", typeof(Terminal)),
                (nameof(Binding), "= 5;", typeof(Binding)),
                (nameof(Element.AST.Type), ":Foo.Bar", typeof(Element.AST.Type)),
                (nameof(Expression), "a.b(c).d.e(5)", typeof(Expression)),
                (nameof(CallExpression), "(5, a(10, c))", typeof(CallExpression)),
                (nameof(IndexingExpression), ".identifier", typeof(IndexingExpression)),
                ($"{nameof(DeclaredFunction)}-ExpressionBody", "a(a):Num = 5;", typeof(DeclaredFunction)),
                ($"{nameof(DeclaredFunction)}-ScopeBody", "a(a):Num {return = 5;}", typeof(DeclaredFunction)),
                ($"Intrinsic {nameof(DeclaredFunction)}", "intrinsic a(a):Num;", typeof(DeclaredFunction)),
                ($"Intrinsic {nameof(DeclaredConstraint)}", "intrinsic constraint a;", typeof(DeclaredConstraint)),
                (nameof(DeclaredConstraint), "constraint a(a):Num;", typeof(DeclaredConstraint)),
                (nameof(DeclaredStruct), "struct a(a);", typeof(DeclaredStruct)),
                ($"{nameof(DeclaredStruct)}-WithScope", "struct a(a) {}", typeof(DeclaredStruct)),
                ($"Intrinsic {nameof(DeclaredStruct)}", "intrinsic struct a(a);", typeof(DeclaredStruct)),
                (nameof(PortList), "(a:foo, b:bar)", typeof(PortList)),
                (nameof(Namespace), "namespace foo {}", typeof(Namespace))
            };
            foreach (var item in data)
            {
                yield return new TestCaseData((item.Item2, item.Item3)).SetName($"Syntax-{item.Item1}");
            }
        }

        // TODO: Make this test use host so that it can test process hosts!
        [TestCaseSource(nameof(PartialSyntaxTestData))]
        public void ParsePartialSyntaxItems((string text, Type syntaxItem) info)
        {
            if(!(_host is AtomicHost)) Assert.Inconclusive("Test only implemented for self host");
            Assert.That(Lexico.Lexico.TryParse(info.text, info.syntaxItem, out _, new ConsoleTrace()), Is.True);
        }
    }
}