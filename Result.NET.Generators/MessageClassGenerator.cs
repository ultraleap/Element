using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;
using Tomlyn;
using Tomlyn.Model;

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
            new DiagnosticDescriptor(DiagId(0),
                "Unhandled exception",
                "Unhandled exception '{0}'",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);
        
        private static readonly DiagnosticDescriptor BadMessageFileNameFormat =
            new DiagnosticDescriptor(DiagId(1),
                "Bad Message File Name Format",
                "Message File Name at '{0}' format is bad - file name format should be 'Namespace.ClassName-messages.toml'",
                nameof(MessageClassGenerator),
                DiagnosticSeverity.Error,
                true);

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
                        context.ReportDiagnostic(Diagnostic.Create(BadMessageFileNameFormat, Location.None, file.Path));
                        continue;
                    }
                    GenerateMessageClass(match.Groups["namespace"].Value,
                        match.Groups["classname"].Value,
                        file.GetText(context.CancellationToken)!,
                        context);
                }
            }
            catch (Exception e)
            {
                context.ReportDiagnostic(Diagnostic.Create(UnhandledException, Location.None, e));
            }
        }

        private static void GenerateMessageClass(string classNamespace, string className, SourceText file, GeneratorExecutionContext context)
        {
            var toml = Toml.Parse(file.ToString()).ToModel();

            var spacesPerIndent = 4;
            void Indent(StringBuilder source, int amount) => source.Append(' ', amount * spacesPerIndent);
            string? prefix = null;

            void AppendMessageInfosByCode(StringBuilder source, int indent)
            {
                foreach (var pair in toml)
                {
                    prefix ??= pair.Key.Substring(0, 3);
                    var code = int.Parse(pair.Key.Substring(3));
                    var table = pair.Value as TomlTable;
                    // Example line
                    // {0, new MessageInfo(MessagePrefix, ""Success"", MessageLevel.Verbose, 0)},
                    Indent(source, indent);
                    source.AppendLine($"{{{code}, new MessageInfo(MessagePrefix, \"{table["name"]}\", MessageLevel.{table["level"]}, {code})}},");
                }
            }

            void AppendMessageInfoProperties(StringBuilder source, int indent)
            {
                foreach (var pair in toml)
                {
                    var code = int.Parse(pair.Key.Substring(3));
                    var table = pair.Value as TomlTable;
                    // Example lines
                    // public static MessageInfo Success => _infoByCode[0];
                    // public static MessageInfo ParseError => _infoByCode[9];
                    Indent(source, indent);
                    source.AppendLine($"public static MessageInfo {table["name"]} => _infoByCode[{code}];");
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
            AppendMessageInfosByCode(source, 4);
            Indent(source, 3);
            source.Append(
            @$"}};
            MessageInfo.GetFuncsByPrefix[MessagePrefix] = GetByCode;
        }}
        
        public static readonly string MessagePrefix = ""{prefix}"";

        public static MessageInfo GetByCode(int code) =>
            _infoByCode.TryGetValue(code, out var info)
                ? info
                : throw new InvalidOperationException($""No info found for message '{{MessagePrefix}}{{code}}' - code was not found"");

");

        AppendMessageInfoProperties(source, 2);
        
        Indent(source, 1);
        source.Append(
    @"}
}");
                
                
            context.AddSource($"{classNamespace}.{className}", source.ToString());
        }
    }
}