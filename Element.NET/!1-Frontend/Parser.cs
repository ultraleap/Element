using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using Element.AST;
using Lexico;

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
    internal static class Parser
    {
        public static Identifier ReturnIdentifier { get; } = new Identifier("return");

        public static readonly Identifier[] GloballyReservedIdentifiers =
        {
            new Identifier("_"),
            new Identifier("intrinsic"),
            new Identifier("namespace"),
            new Identifier("struct"),
            ReturnIdentifier,
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

        private static string Preprocess(string text) => Regex.Replace(text, @"#.*", string.Empty, RegexOptions.Multiline | RegexOptions.Compiled);

        public static bool Parse<T>(this Context context, string text, out T output)
        {
            var parseTrace = new List<string>();
            var success = Lexico.Lexico.TryParse(text, out output, new DelegateTextTrace(msg =>
            {
                if (!string.IsNullOrEmpty(msg)) parseTrace.Add(msg);
            }));
            if (!success)
            {
                foreach (var msg in parseTrace)
                {
                    context.Log(msg);
                }
                context.LogError(9, "Parsing failed, see previous parse trace messages for details.");
            }
            else if (context.Verbosity >= MessageLevel.Information)
            {
                foreach (var msg in parseTrace)
                {
                    context.Log(msg);
                }
            }
            return success;
        }

        public static bool ValidateIdentifier(this SourceContext context, Identifier identifier, Identifier[] blacklist = null, Identifier[] whitelist = null)
        {
            if (string.IsNullOrEmpty(identifier))
            {
                context.LogError(15, "Null or empty identifier is invalid");
                return false;
            }

            bool Predicate(Identifier reserved) => string.Equals(identifier, reserved, StringComparison.OrdinalIgnoreCase);

            if (GloballyReservedIdentifiers.Where(id => !(whitelist?.Contains(identifier) ?? false)).Any(Predicate)
                || (blacklist?.Any(Predicate) ?? false))
            {
                context.LogError(15, $"'{identifier}' is a reserved identifier");
                return false;
            }

            return true;
        }

        /// <summary>
        /// Parses the given file as an Element source file and adds it's contents to a source context
        /// </summary>
        public static bool ParseFile(this SourceContext context, FileInfo file) => ParseFile(context, file, true);


        private static bool ParseFile(this SourceContext context, FileInfo file, bool validate)
        {
            var success = context.Parse<SourceScope>(Preprocess(File.ReadAllText(file.FullName)), out var sourceScope);
            if (success)
            {
                context.GlobalScope[file] = sourceScope;
                context.GlobalScope.InitializeItems();
                if (validate)
                {
                    success &= context.GlobalScope.ValidateScope(context);
                }
            }

            return success;
        }


        /// <summary>
        /// Parses all the given files as Element source files into the compilation context
        /// </summary>
        public static (bool OverallSuccess, IEnumerable<(bool Success, FileInfo FileInfo)> Results) ParseFiles(this SourceContext context,
            IEnumerable<FileInfo> files)
        {
            (bool Success, FileInfo File)[] fileResults = files.Where(file => context.GlobalScope[file] == null).Select(file => (context.ParseFile(file, false), file)).ToArray();
            var overallSuccess = fileResults.All(fr => fr.Success) && context.GlobalScope.ValidateScope(context);
            return (overallSuccess, fileResults);
        }
    }
}