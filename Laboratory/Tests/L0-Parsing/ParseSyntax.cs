using System;
using System.Collections;
using System.Collections.Generic;
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
                (nameof(Terminal), ";", typeof(Terminal)),
                (nameof(ExpressionBody), "= 5", typeof(ExpressionBody)),
                (nameof(Binding), "= 5;", typeof(Binding)),
                (nameof(TypeAnnotation), ":Foo.Bar", typeof(TypeAnnotation)),
                (nameof(Expression), "a.b(c).d.e(5)", typeof(Expression)),
                (nameof(CallExpression), "(5, a(10, c))", typeof(CallExpression)),
                (nameof(IndexingExpression), ".identifier", typeof(IndexingExpression)),
                (nameof(Lambda), "_(_a) = 5", typeof(Lambda)),
                ($"{nameof(CustomFunctionSignatureDeclaration)}-ExpressionBody", "a(a):Num = 5;", typeof(CustomFunctionSignatureDeclaration)),
                ($"{nameof(CustomFunctionSignatureDeclaration)}-ScopeBody", "a(a):Num {return = 5;}", typeof(CustomFunctionSignatureDeclaration)),
                ($"{nameof(IntrinsicFunctionSignatureDeclaration)}", "intrinsic a(a):Num;", typeof(IntrinsicFunctionSignatureDeclaration)),
                (nameof(CustomConstraintDeclaration), "constraint a(a):Num;", typeof(CustomConstraintDeclaration)),
                ($"{nameof(IntrinsicConstraintDeclaration)}", "intrinsic constraint a;", typeof(IntrinsicConstraintDeclaration)),
                (nameof(CustomStructDeclaration), "struct a(a);", typeof(CustomStructDeclaration)),
                ($"{nameof(CustomStructDeclaration)}-WithScope", "struct a(a) {}", typeof(CustomStructDeclaration)),
                ($"{nameof(IntrinsicStructDeclaration)}", "intrinsic struct a(a);", typeof(IntrinsicStructDeclaration)),
                (nameof(Port), "a:foo", typeof(Port)),
                (nameof(PortList), "(a:foo, b:bar)", typeof(PortList)),
                (nameof(NamespaceDeclaration), "namespace foo {}", typeof(NamespaceDeclaration)),
            };
            foreach (var (name, partialExpression, type) in data)
            {
                yield return new TestCaseData((partialExpression, type)).SetName($"Syntax-{name}");
            }
        }

        [TestCaseSource(nameof(GenerateParseTestData))]
        public void Parse((FileInfo fileInfo, MessageCode? messageCode) info) => SyntaxTest(info, true);
        
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