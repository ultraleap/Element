using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

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
    /// Provides methods to convert text into 
    /// </summary>
    public static class Parser
    {
        public static readonly string[] GloballyReservedIdentifiers =
        {
            "_",
            "any",
            "intrinsic",
            "namespace",
            "return",
            "struct"
        };

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

        private static string Preprocess(string text) =>
            Regex.Replace(text, @"#.*", string.Empty, RegexOptions.Multiline | RegexOptions.Compiled);

        private static bool LexicoParseFile(this CompilationContext context, FileInfo info)
        {
            string traceOutput = null;
            var success = Lexico.Parser.TryParse<GlobalScope>(Preprocess(File.ReadAllText(info.FullName)),
                                                                      out var output,
                                                                      trace => traceOutput = trace);
            if (success)
            {
                if (!string.IsNullOrEmpty(traceOutput))
                {
                    context.Log(traceOutput);
                }
            }
            else
            {
                context.LogError(9, traceOutput);
            }
            return success;
        }

        /// <summary>
        /// Parses the given file as an Element source file and adds it's contents to a global scope
        /// </summary>
        public static bool ParseFile(this CompilationContext context, FileInfo file) => LexicoParseFile(context, file);

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