using System;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using Lexico;

namespace Element.AST
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

        public static Result<T> Parse<T>(SourceInfo source, Context context, bool noParseTrace = false) where T : notnull
        {
            if (Lexico.Lexico.TryParse(source.PreprocessedText, out T output, userObject: source))
                return new Result<T>(output, context.Trace(MessageLevel.Information, 
                                                           $"Parsed '{source.Name}' \"{source.FirstNonEmptyLine}\" - successfully"));
            if (noParseTrace) return context.Trace(MessageCode.ParseError, $"Parsing failed within source '{source.Name}' - enable parse trace and run again for details.");
            
            // Using StringBuilder as there's potentially a lot of trace lines
            var sb = new StringBuilder();
            Lexico.Lexico.TryParse<T>(source.PreprocessedText, out _, new DelegateTextTrace(msg => { if (!string.IsNullOrEmpty(msg)) sb.AppendLine(msg); }), source);
            return context.Trace(MessageCode.ParseError, $"Parsing failed within '{source.Name}' - see parse trace below for details.\n{sb}");
        }

        public static void Validate(this Identifier identifier, ResultBuilder builder, Identifier[] blacklist, Identifier[] whitelist)
        {
            if (string.IsNullOrEmpty(identifier.String))
            {
                builder.Append(MessageCode.InvalidIdentifier, "Null or empty identifier is invalid");
            }

            bool Predicate(Identifier reserved) => string.Equals(identifier.String, reserved.String, StringComparison.OrdinalIgnoreCase);
            if (GloballyReservedIdentifiers.Except(whitelist).Any(Predicate) || blacklist.Any(Predicate))
            {
                builder.Append(MessageCode.InvalidIdentifier, $"'{identifier}' is a reserved identifier");
            }
        }
    }
}