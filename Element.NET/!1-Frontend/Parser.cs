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

        public static bool Parse<T>(string text, out T output, ILogger logger, bool noParseTrace = false)
        {
            var success = Lexico.Lexico.TryParse(text, out output);
            if (!success)
            {
                if (!noParseTrace)
                {
                    var sb = new StringBuilder();
                    Lexico.Lexico.TryParse<T>(text, out _, new DelegateTextTrace(msg =>
                    {
                        if (!string.IsNullOrEmpty(msg)) sb.AppendLine(msg);
                    }));
                    logger.Log(sb.ToString());
                }

                logger.LogError(9, $"Parsing failed - {(noParseTrace ? "enable parse trace and run again for details." : "see parse trace messages for details.")}");
            }
            return success;
        }

        public static bool ValidateIdentifier(Identifier identifier, ILogger logger, Identifier[]? blacklist = null, Identifier[]? whitelist = null)
        {
            if (string.IsNullOrEmpty(identifier))
            {
                logger.LogError(15, "Null or empty identifier is invalid");
                return false;
            }

            bool Predicate(Identifier reserved) => string.Equals(identifier, reserved, StringComparison.OrdinalIgnoreCase);

            if (GloballyReservedIdentifiers.Where(id => !(whitelist?.Contains(identifier) ?? false)).Any(Predicate)
                || (blacklist?.Any(Predicate) ?? false))
            {
                logger.LogError(15, $"'{identifier}' is a reserved identifier");
                return false;
            }

            return true;
        }
    }
}