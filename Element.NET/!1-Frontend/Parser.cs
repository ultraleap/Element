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

        public static string Preprocess(string text) => Regex.Replace(text, @"#.*", string.Empty, RegexOptions.Multiline | RegexOptions.Compiled);

        public static Result<T> Parse<T>(string text, ITrace trace, bool noParseTrace = false) where T : notnull
        {
            if (Lexico.Lexico.TryParse(text, out T output)) return new Result<T>(output);
            if (noParseTrace) return trace.Trace(MessageCode.ParseError, "Parsing failed - enable parse trace and run again for details.");
            
            // Using StringBuilder as there's potentially a lot of trace lines
            var sb = new StringBuilder();
            Lexico.Lexico.TryParse<T>(text, out _, new DelegateTextTrace(msg => { if (!string.IsNullOrEmpty(msg)) sb.AppendLine(msg); }));
            return trace.Trace(MessageCode.ParseError, $"Parsing failed - see parse trace below for details.\n{sb}");
        }

        public static void Validate(this Identifier identifier, ResultBuilder builder, Identifier[]? blacklist = null, Identifier[]? whitelist = null)
        {
            if (string.IsNullOrEmpty(identifier))
            {
                builder.Append(MessageCode.InvalidIdentifier, "Null or empty identifier is invalid");
            }

            bool Predicate(Identifier reserved) => string.Equals(identifier, reserved, StringComparison.OrdinalIgnoreCase);
            if (GloballyReservedIdentifiers.Where(id => !(whitelist?.Contains(identifier) ?? false)).Any(Predicate)
                || (blacklist?.Any(Predicate) ?? false))
            {
                builder.Append(MessageCode.InvalidIdentifier, $"'{identifier}' is a reserved identifier");
            }
        }
    }
}