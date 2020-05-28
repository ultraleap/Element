using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using Newtonsoft.Json;
using Tomlyn;
using Tomlyn.Model;

namespace Element
{
    public readonly struct CompilerMessage
    {
        private static readonly TomlTable _messageToml = Toml.Parse(File.ReadAllText("Messages.toml")).ToModel();

        private static bool TryGetMessageToml(int messageCode, out TomlTable message)
        {
            message = _messageToml.TryGetToml($"ELE{messageCode}", out var obj) && obj is TomlTable table ? table : null;
            return message != null;
        }
        
        public static bool TryGetMessageName(int messageCode, out string name)
        {
            name = TryGetMessageToml(messageCode, out var table) ? table["name"] as string : null;
            return name != null;
        }

        public static bool TryGetMessageLevel(int messageCode, out MessageLevel level)
        {
            var name = TryGetMessageToml(messageCode, out var table) ? table["level"] as string: null;
            return Enum.TryParse(name, out level);
        }

        public CompilerMessage(string message, MessageLevel? messageLevel = null) : this(null, messageLevel, message, null){}

        [JsonConstructor]
        public CompilerMessage(int? messageCode, MessageLevel? messageLevel, string? context, IReadOnlyCollection<TraceSite>? traceStack)
        {
            MessageCode = messageCode;
            MessageLevel = Element.MessageLevel.Information;
            Context = context;
            TraceStack = traceStack ?? Array.Empty<TraceSite>();

            var builder = new StringBuilder();
            if (messageCode.HasValue)
            {
                MessageLevel = TryGetMessageLevel(messageCode.Value, out var level) ? level : Element.MessageLevel.Information;
                builder.Append("ELE").Append(MessageCode).Append(": ").Append(MessageLevel).Append(" - ").AppendLine(TryGetMessageName(messageCode.Value, out var message) ? message : "Unknown");
            }

            builder.Append(Context);
            if (TraceStack.Count > 0)
            {
                builder.AppendLine();
                builder.AppendLine("Element source trace:");
                foreach (var site in TraceStack)
                {
                    builder.Append("    ").AppendLine(site.ToString());
                }

                builder.AppendLine();
            }

            _message = builder.ToString();
        }
        private readonly string _message;

        public int? MessageCode { get; }
        public MessageLevel? MessageLevel { get; }
        public string? Context { get; }
        public IReadOnlyCollection<TraceSite> TraceStack { get; }

        public override string ToString() => _message;
    }
}