using System;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using Lexico;

namespace Element.AST
{
    /// <summary>
    /// Provides methods to parse source text into AST objects.
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
                return new Result<T>(output, context.Trace(MessageLevel.Information, $"Parsed '{source.DisplayName}' successfully"));
            if (noParseTrace) return context.Trace(EleMessageCode.ParseError, $"Parsing failed within source '{source.FullName}' - enable parse trace and run again for details.");
            
            // Using StringBuilder as there's potentially a lot of trace lines
            var sb = new StringBuilder();
            Lexico.Lexico.TryParse<T>(source.PreprocessedText, out _, new DelegateTextTrace(msg => { if (!string.IsNullOrEmpty(msg)) sb.AppendLine(msg); }), source);
            return context.Trace(EleMessageCode.ParseError, $"Parsing failed within '{source.DisplayName}' - see parse trace below for details.\n{sb}");
        }

        public static void Validate(this Identifier identifier, ResultBuilder builder, Identifier[] blacklist, Identifier[] whitelist)
        {
            if (string.IsNullOrEmpty(identifier.String))
            {
                builder.Append(EleMessageCode.InvalidIdentifier, "Null or empty identifier is invalid");
            }

            bool Predicate(Identifier reserved) => string.Equals(identifier.String, reserved.String, StringComparison.OrdinalIgnoreCase);
            if (GloballyReservedIdentifiers.Except(whitelist).Any(Predicate) || blacklist.Any(Predicate))
            {
                builder.Append(EleMessageCode.InvalidIdentifier, $"'{identifier}' is a reserved identifier");
            }
        }
    }
    
    [TopLevel]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class TopLevel<T>
    {
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
#pragma warning disable 8618
        [Term] public T Object { get; private set; }
#pragma warning restore 8618
    }
}