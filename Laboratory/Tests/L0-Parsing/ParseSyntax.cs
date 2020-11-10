using System;
using System.Collections;
using System.IO;
using Element;
using Element.AST;
using Lexico;
using NUnit.Framework;
using Expression = Element.AST.Expression;

namespace Laboratory.Tests.L0.Parsing
{
    internal class ParseSyntax : SyntaxFixture
    {
        private static IEnumerable GenerateParseTestData() => GenerateTestData("Parse", "L0-Parsing", DefaultFailingParseTestCode);
        
        private static IEnumerable PartialSyntaxTestData()
        {
            var data = new[]
            {
                (nameof(Identifier), "a", typeof(Identifier)),
                (nameof(Unidentifier), "_", typeof(Unidentifier)),
                (nameof(Constant), "5", typeof(Constant)),
                (nameof(Nothing), "", typeof(Nothing)),
                (nameof(ExpressionBody), "= 5", typeof(ExpressionBody)),
                ("Empty Block", "{ }", typeof(FreeformBlock)),
                ("Block with Declarations", "{ a = 5, b = 10 }", typeof(CommaSeparatedBlock)),
                (nameof(PortConstraint), ":Foo.Bar", typeof(PortConstraint)),
                (nameof(Expression), "a.b(c).d.e(5)", typeof(Expression)),
                (nameof(ExpressionChain.CallExpression), "(5, a(10, c))", typeof(ExpressionChain.CallExpression)),
                (nameof(ExpressionChain.IndexingExpression), ".identifier", typeof(ExpressionChain.IndexingExpression)),
                (nameof(Lambda), "_(_a) = 5", typeof(Lambda)),
                ($"{nameof(Lambda)}-InExpression", "a = _(_a) = 5", typeof(ExpressionBodiedFunctionDeclaration)),
                (nameof(AnonymousBlock), "{b = 5, c = 10}", typeof(AnonymousBlock)),
                ($"{nameof(AnonymousBlock)}-InExpression", "a = {b = 5, c = 10}", typeof(ExpressionBodiedFunctionDeclaration)),
                ($"{nameof(ExpressionBodiedFunctionDeclaration)}", "a(a):Num = 5", typeof(ExpressionBodiedFunctionDeclaration)),
                ($"{nameof(ScopeBodiedFunctionDeclaration)}", "a(a):Num {return = 5}", typeof(ScopeBodiedFunctionDeclaration)),
                ($"{nameof(IntrinsicFunctionDeclaration)}", "intrinsic function a(a):Num", typeof(IntrinsicFunctionDeclaration)),
                (nameof(CustomConstraintDeclaration), "constraint a(a):Num", typeof(CustomConstraintDeclaration)),
                ($"{nameof(IntrinsicConstraintDeclaration)}", "intrinsic constraint a", typeof(IntrinsicConstraintDeclaration)),
                (nameof(CustomStructDeclaration), "struct a(a)", typeof(CustomStructDeclaration)),
                ($"{nameof(CustomStructDeclaration)}-WithAssociatedScope", "struct a(a) {}", typeof(CustomStructDeclaration)),
                ($"{nameof(IntrinsicStructDeclaration)}", "intrinsic struct a(a)", typeof(IntrinsicStructDeclaration)),
                (nameof(Port), "a:foo", typeof(Port)),
                ($"{nameof(Port)}-WithDefaultArgument", "a:foo = 15", typeof(Port)),
                (nameof(PortList), "(a:foo, b:bar)", typeof(PortList)),
                ($"{nameof(PortList)}-WithDefaultArguments", "(a:foo = 15, b:bar = Vector3(1, 2, 3))", typeof(PortList)),
                (nameof(NamespaceDeclaration), "namespace foo {}", typeof(NamespaceDeclaration)),
            };
            foreach (var (name, partialExpression, type) in data)
            {
                yield return new TestCaseData((partialExpression, type)).SetName($"Syntax-{name}");
            }
        }

        [TestCaseSource(nameof(GenerateParseTestData))]
        public void Parse((FileInfo fileInfo, EleMessageCode? messageCode) info) => SyntaxTest(info, true);
        
        // TODO: Force consuming the whole text in all of these parsers
        // TODO: Make this test use an IHost so that it can test process hosts!
        [TestCaseSource(nameof(PartialSyntaxTestData))]
        public void ParsePartialSyntaxItems((string text, Type syntaxItem) info)
        {
            if(!(Host is AtomicHost)) Assert.Inconclusive("Test only implemented for self host");
            Assert.That(Lexico.Lexico.TryParse(info.text, info.syntaxItem, out _, new ConsoleTrace()), Is.True);
        }
    }
}