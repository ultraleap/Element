using System;
using System.Collections.Generic;
using System.Linq;
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

        public static bool Parse<T>(this Context context, string text, out T output)
        {
            var success = Lexico.Lexico.TryParse(text, out output);
            if (!success)
            {
                var parseTrace = new List<string>();
                Lexico.Lexico.TryParse<T>(text, out _, new DelegateTextTrace(msg =>
                {
                    if (!string.IsNullOrEmpty(msg)) parseTrace.Add(msg);
                }));
                foreach (var msg in parseTrace)
                {
                    context.Log(msg);
                }
                context.LogError(9, "Parsing failed, see previous parse trace messages for details.");
            }
            return success;
        }

        public static bool ValidateIdentifier(this Context context, Identifier identifier, Identifier[] blacklist = null, Identifier[] whitelist = null)
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
    }
}