using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;
using Tomlyn;
using Tomlyn.Model;

namespace System.Runtime.CompilerServices
{
    // Fix to allow record type primary constructor to work with older than .NET 5.0
    // See https://stackoverflow.com/questions/64749385/predefined-type-system-runtime-compilerservices-isexternalinit-is-not-defined
    internal static class IsExternalInit {}
}

namespace Result.NET.Generators
{
    [Generator]
    public class MessageClassGenerator : ISourceGenerator
    {
        public void Initialize(GeneratorInitializationContext context)
        {
            //Debugger.Launch();
        }
        
        private static string DiagId(int num) => $"MsgCodeGen{num}";
        
        private static readonly DiagnosticDescriptor UnhandledException =
            // ReSharper disable RS1017
            new DiagnosticDescriptor(DiagId(0),
                "Unhandled exception",
                "Unhandled exception '{0}'",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);
        
        private static readonly DiagnosticDescriptor MessageFileNameBadFormat =
            new DiagnosticDescriptor(DiagId(1),
                "Message File Name Bad Format",
                "Message File Name Bad Format: Expected file name of format '<Namespace>.<ClassName>-messages.toml' but got {0}",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);
        
        private static readonly DiagnosticDescriptor TomlParseError =
            new DiagnosticDescriptor(DiagId(2),
                "Toml File Parse Error",
                "Toml File Parse Error: '{0}' - {1}",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);
        
        private static readonly DiagnosticDescriptor TomlMessageCodeBadFormat =
            new DiagnosticDescriptor(DiagId(3),
                "Toml Message Code Bad Format",
                "Toml Message Code Bad Format: '{0}' format must be 3 uppercase letters and a unique index 'ABC2021'",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);
        
        private static readonly DiagnosticDescriptor TomlMessageInfoMissingKey =
            new DiagnosticDescriptor(DiagId(4),
                "Toml Message Info Missing Key",
                "Toml Message Info Missing Key: Missing '{0}' for '{1}'",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);
        
        private static readonly DiagnosticDescriptor TomlMessageInfoValueWrongType =
            new DiagnosticDescriptor(DiagId(5),
                "Toml Message Info Value Wrong Type",
                "Toml Message Info Value Wrong Type: Expected {0} but got {1} for '{2}.{3}'",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);
        
        private static readonly DiagnosticDescriptor TomlMessageInfoValueInvalid =
            new DiagnosticDescriptor(DiagId(6),
                "Toml Message Info Value Invalid",
                "Toml Message Info Value Invalid: Invalid Value '{0}' for '{1}.{2}'",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);
        
        private static readonly DiagnosticDescriptor TomlDuplicateMessageInfo =
            new DiagnosticDescriptor(DiagId(7),
                "Toml Duplicate Message Info",
                "Toml Duplicate Message Info: Duplicate definitions of '{0}'",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);
        
        private static readonly DiagnosticDescriptor TomlMultipleMessagePrefixes =
            new DiagnosticDescriptor(DiagId(8),
                "Toml Multiple Message Prefixes",
                "Toml Multiple Message Prefixes: Expected prefix {0} but got {1} - each messages file should define messages for a single prefix",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);
        // ReSharper restore RS1017
        
        // TODO: Reference Result.NET instead of having these declared here
        //       At time of writing it appears there's no facility to reference a project rather than nuget package as a generation-time dependency
        private record MessageInfo(string Prefix, string Name, MessageLevel Level, int Code);

        private enum MessageLevel
        {
            Verbose = -1,
            Information = 0, // Information is the default level
            Warning,
            Error,
        }

        private static readonly string TomlNameFieldKey = "name";
        private static readonly string TomlLevelFieldKey = "level";

        public void Execute(GeneratorExecutionContext context)
        {
            try
            {
                foreach(var file in context.AdditionalFiles.Where(f => f.Path.EndsWith("-messages.toml")))
                {
                    var fileName = Path.GetFileName(file.Path);
                    var match = Regex.Match(fileName, @"^(?<namespace>\w+)\.(?<classname>\w+)-messages\.toml$");
                    if (!match.Success)
                    {
                        context.ReportDiagnostic(Diagnostic.Create(MessageFileNameBadFormat, Location.None, fileName));
                        continue;
                    }
                    GenerateMessageClass(match.Groups["namespace"].Value,
                        match.Groups["classname"].Value,
                        file.Path,
                        file.GetText(context.CancellationToken)!,
                        context);
                }
            }
            catch (Exception e)
            {
                context.ReportDiagnostic(Diagnostic.Create(UnhandledException, Location.None, e));
            }
        }

        private static void GenerateMessageClass(string classNamespace, string className, string filePath, SourceText file, GeneratorExecutionContext context)
        {
            TomlTable toml;
            try
            {
                toml = Toml.Parse(file.ToString()).ToModel();
            }
            catch (Exception e)
            {
                context.ReportDiagnostic(Diagnostic.Create(TomlParseError, Location.None, filePath, e));
                return;
            }

            var spacesPerIndent = 4;
            void Indent(StringBuilder source, int amount) => source.Append(' ', amount * spacesPerIndent);
            string? fileMessagePrefix = null;
            HashSet<string> messageInfoKeys = new();

            var infos = toml.Select(pair =>
            {
                var messageTableKey = Regex.Match(pair.Key, @"^(?<prefix>[A-Z]{3})(?<code>\d+)$");
                if (!messageTableKey.Success)
                {
                    context.ReportDiagnostic(Diagnostic.Create(TomlMessageCodeBadFormat, Location.None, pair.Key));
                    return default;
                }

                if (!messageInfoKeys.Add(pair.Key))
                {
                    context.ReportDiagnostic(Diagnostic.Create(TomlDuplicateMessageInfo, Location.None, pair.Key));
                    return default;
                }

                var prefix = messageTableKey.Groups["prefix"].Value;
                fileMessagePrefix ??= prefix; // Assign the first prefix to be the known prefix for the file
                if (!fileMessagePrefix.Equals(prefix))
                {
                    context.ReportDiagnostic(Diagnostic.Create(TomlMultipleMessagePrefixes, Location.None, fileMessagePrefix, prefix));
                    return default;
                }
                var code = int.Parse(messageTableKey.Groups["code"].Value); // This will definitely succeed due to the regex above
                
                var table = (TomlTable)pair.Value;

                bool TryGetTomlValue<T>(string key, ObjectKind kindForType, out T value)
                {
                    if(!table.TryGetToml(key, out var tomlObject))
                    {
                        context.ReportDiagnostic(Diagnostic.Create(TomlMessageInfoMissingKey, Location.None, key, pair.Key));
                        value = default!;
                        return false;
                    }

                    if (tomlObject.Kind != kindForType)
                    {
                        context.ReportDiagnostic(Diagnostic.Create(TomlMessageInfoValueWrongType, Location.None, kindForType, tomlObject.Kind, pair.Key, key));
                        value = default!;
                        return false;
                    }

                    var obj = table[key]; // We re-index the table because TomlObject.ToObject is internal in Tomlyn and this is the easiest way to use it
                    if (obj is not T v)
                    {
                        context.ReportDiagnostic(Diagnostic.Create(TomlMessageInfoValueWrongType, Location.None, typeof(T), obj.GetType(), pair.Key, key));
                        value = default!;
                        return false;
                    }

                    value = v;
                    return true;
                }

                if (!TryGetTomlValue(TomlNameFieldKey, ObjectKind.String, out string name)
                    || !TryGetTomlValue(TomlLevelFieldKey, ObjectKind.String, out string levelString))
                {
                    return default;
                }
                
                if (!Enum.TryParse(levelString, out MessageLevel messageLevel))
                {
                    context.ReportDiagnostic(Diagnostic.Create(TomlMessageInfoValueInvalid, Location.None, levelString, pair.Key, TomlLevelFieldKey));
                    return default;
                }

                return new MessageInfo(prefix, name, messageLevel, code);
            }).ToArray();
            if (infos.Any(i => i == default)) return; // Diagnostic will already be reported

            void AppendMessageInfosLines(StringBuilder builder, int indent, Func<MessageInfo, string> makeLine)
            {
                foreach (var info in infos)
                {
                    Indent(builder, indent);
                    builder.AppendLine(makeLine(info!));
                }
            }
            
            var source = new StringBuilder(
$@"using System;
using System.Collections.Generic;
using ResultNET;

namespace {classNamespace}
{{
    public static class {className}
    {{
        private static readonly Dictionary<int, MessageInfo> _infoByCode;
            
        static {className}()
        {{
            _infoByCode = new Dictionary<int, MessageInfo>
            {{
");
            // Example line
            // {0, new MessageInfo(MessagePrefix, ""Success"", MessageLevel.Verbose, 0)},
            AppendMessageInfosLines(source, 4, info => $"{{{info.Code}, new MessageInfo(MessagePrefix, \"{info.Name}\", MessageLevel.{info.Level}, {info.Code})}},");
            Indent(source, 3);
            source.Append(
            @$"}};
            MessageInfo.GetFuncsByPrefix[MessagePrefix] = GetByCode;
        }}
        
        public static readonly string MessagePrefix = ""{fileMessagePrefix}"";

        public static MessageInfo GetByCode(int code) =>
            _infoByCode.TryGetValue(code, out var info)
                ? info
                : throw new InvalidOperationException($""No info found for message '{{MessagePrefix}}{{code}}' - code was not found"");

");
        // Example line
        // public static MessageInfo Success => _infoByCode[0];
        AppendMessageInfosLines(source, 2, info => $"public static MessageInfo {info.Name} => _infoByCode[{info.Code}];");
        
        Indent(source, 1);
        source.Append(
    @"}
}");
                
                
            context.AddSource($"{classNamespace}.{className}", source.ToString());
        }
    }
}