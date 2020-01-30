using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using Eto.Parse;
using Eto.Parse.Grammars;

namespace Element
{
    public static class ElementAST
    {
        // Expressions - the most basic components which everything else is composed by
        public const string LiteralExpression = "lit";
        public const string VariableExpression = "var";
        public const string SubExpression = "sub";
        public const string CallExpression = "call";

        // Expression-specific ElementAST elements
        public const string SubExpressionRoot = "subroot";
        public const string SubExpressionName = "subname";
        public const string Callee = "callee";
        public const string CallArguments = "args";
        public const string CallArgument = "arg";

        // Ports - defines syntax for a particular input or output from a function
        public const string Port = "port";
        public const string PortType = "porttype";
        public const string PortName = "portname";

        // Functions - all components for a function
        public const string FunctionName = "name";
        public const string FunctionInputs = "inputs";
        public const string FunctionOutputs = "outputs";
        public const string FunctionBody = "body";

        // Statement - all assignments and function declarations are statements
        public const string Binding = "bind";
        public const string TypeStatement = "type";
        public const string Declaration = "decl";
    }

    /// <summary>
    /// Provides methods to convert text into Functions
    /// </summary>
    public static class Parser
    {
        private static readonly char _lineCommentCharacter = '#';

        private static string ElementEbnf { get; } = File.ReadAllText("Grammar.ebnf");

        private static Grammar ElementGrammar =>
            new EbnfGrammar(EbnfStyle.W3c | EbnfStyle.WhitespaceSeparator)
                .Build(ElementEbnf, "grammar");

        internal static (int Line, int Column, int LineCharacterIndex) CountLinesAndColumns(int index, string text)
        {
            var line = 1;
            var column = 1;
            var lineCharacterIndex = 0;
            for (var i = 0; i < index; i++)
            {
                if (text[i] == '\r')
                {
                    line++;
                    column = 1;
                    lineCharacterIndex = 0;
                    if (i + 1 < text.Length && text[i + 1] == '\n')
                    {
                        i++;
                    }
                }
                else if (text[i] == '\n')
                {
                    line++;
                    column = 1;
                    lineCharacterIndex = 0;
                }
                else if (text[i] == '\t')
                {
                    column += 4;
                    lineCharacterIndex++;
                }
                else
                {
                    column++;
                    lineCharacterIndex++;
                }
            }

            return (line, column, lineCharacterIndex);
        }


        private static string Preprocess(string text)
        {
            using var input = new StringReader(text);
            using var output = new StringWriter();
            while (true)
            {
                var line = input.ReadLine();
                if (line != null)
                {
                    var lineCommentIdx = line.IndexOf(_lineCommentCharacter);
                    output.WriteLine(lineCommentIdx >= 0 ? line.Substring(0, lineCommentIdx) : line);
                }
                else
                {
                    return output.ToString();
                }
            }
        }

        private static GrammarMatch Parse(this CompilationContext compilationContext, string text, string source = default)
        {
            var preprocessedText = Preprocess(text);
            var match = ElementGrammar.Match(preprocessedText);
            if (!match.Success)
            {
                var builder = new StringBuilder();
                var lines = Regex.Split(preprocessedText, "\r\n|\r|\n");

                void AppendError(int index)
                {
                    if (index < 0) return;
                    var (line, column, lineCharacterIndex) = CountLinesAndColumns(index, preprocessedText);

                    builder.AppendFormat("    in {0}:{1},{2}", source ?? "<no source specified>", line, column);
                    builder.AppendLine();
                    builder.AppendLine();
                    builder.AppendLine(lines[line - 1]);
                    builder.AppendLine(new string(' ', lineCharacterIndex) + "^");
                }
                AppendError(match.ErrorIndex);
                if(match.ChildErrorIndex != match.ErrorIndex) AppendError(match.ChildErrorIndex);

                builder.AppendLine();
                builder.AppendLine("Expected one of the following:");

                foreach (var parser in match.Errors)
                {
                    builder.AppendLine($"    {parser.GetErrorMessage()}");
                }

                compilationContext.LogError(9, builder.ToString());
            }

            return match;
        }

        /// <summary>
        /// Parses the given file as an Element source file and adds it's contents to a global scope
        /// </summary>
        public static bool ParseFile(this CompilationContext context, FileInfo file) => context.Parse(
            File.ReadAllText(file.FullName), file.FullName).Matches.Aggregate(true,
            (current, item) =>
                current
                && (!item.HasMatches // if we have no matches, do nothing
                    || context.GlobalScope.AddParseMatch(item["identifier", true]?.Text,
                        new ParseMatch(item, file), context)));

        /// <summary>
        /// Parses all the given files as Element source files into a global scope
        /// </summary>
        public static IEnumerable<(bool Success, FileInfo FileInfo)> ParseFiles(this CompilationContext context,
            IEnumerable<FileInfo> files) =>
            files.Select(file => (context.ParseFile(file), file)).ToArray();

        /*/// <summary>
        /// Parses the given text as a single Element function using a scope without adding it to the scope
        /// </summary>
        public static IFunction ParseText(this CompilationContext context, string text, IScope scope) =>
            context.Parse(text).ToFunction(scope, null, context);*/
    }
}