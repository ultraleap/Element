using System;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using Element.AST;
using Lexico;

namespace Element
{
    /// <summary>
    /// Provides methods to convert text into 
    /// </summary>
    public static class Parser
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

        public static string Preprocess(string text) => Regex.Replace(text, @"#.*", string.Empty, RegexOptions.Multiline | RegexOptions.Compiled);

        public static Result<T> Parse<T>(SourceInfo source, ITrace trace, bool noParseTrace = false) where T : notnull
        {
            if (Lexico.Lexico.TryParse(source.PreprocessedText, out T output, userObject: source)) return new Result<T>(output);
            if (noParseTrace) return trace.Trace(MessageCode.ParseError, "Parsing failed - enable parse trace and run again for details.");
            
            // Using StringBuilder as there's potentially a lot of trace lines
            var sb = new StringBuilder();
            Lexico.Lexico.TryParse<T>(source.PreprocessedText, out _, new DelegateTextTrace(msg => { if (!string.IsNullOrEmpty(msg)) sb.AppendLine(msg); }), source);
            return trace.Trace(MessageCode.ParseError, $"Parsing failed - see parse trace below for details.\n{sb}");
        }

        public static void Validate(this Identifier identifier, ResultBuilder builder, Identifier[] blacklist, Identifier[] whitelist)
        {
            if (string.IsNullOrEmpty(identifier))
            {
                builder.Append(MessageCode.InvalidIdentifier, "Null or empty identifier is invalid");
            }

            bool Predicate(Identifier reserved) => string.Equals(identifier, reserved, StringComparison.OrdinalIgnoreCase);
            if (GloballyReservedIdentifiers.Except(whitelist).Any(Predicate) || blacklist.Any(Predicate))
            {
                builder.Append(MessageCode.InvalidIdentifier, $"'{identifier}' is a reserved identifier");
            }
        }
    }
}