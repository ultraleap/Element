using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Newtonsoft.Json;
using Tomlyn;
using Tomlyn.Model;

namespace Element
{
    public class CompilerMessage
    {
        private static readonly Dictionary<string, TomlTable> _messageToml = new Dictionary<string, TomlTable>();

        private static bool TryGetMessageToml(string messageType, int messageCode, out TomlTable? message)
        {
            if (string.IsNullOrEmpty(messageType))
            {
                message = null;
                return false;
            }
            if (!_messageToml.TryGetValue(messageType, out var tomlFile))
            {
                lock (_messageToml)
                {
                    _messageToml[messageType] = tomlFile = Toml.Parse(File.ReadAllText($"Messages-{messageType}.toml")).ToModel();
                }
            }
            message = tomlFile.TryGetToml($"{messageType}{messageCode}", out var obj) && obj is TomlTable table ? table : null;
            return message != null;
        }
        
        public static bool TryGetMessageName(string messageType, int messageCode, out string? name)
        {
            name = TryGetMessageToml(messageType, messageCode, out var table) ? (string) table!["name"] : null;
            return name != null;
        }

        public static bool TryGetMessageLevel(string messageType, int messageCode, out MessageLevel level)
        {
            if (Enum.TryParse(TryGetMessageToml(messageType, messageCode, out var table) ? (string) table!["level"] : string.Empty, out level)) return true;
            level = Element.MessageLevel.Error;
            return false;
        }
        
        public CompilerMessage(string message, string messageType = "", MessageLevel? messageLevel = null) : this(messageType, null, messageLevel, message, null) {}
        public CompilerMessage(string messageType, int messageCode, string? context, IReadOnlyCollection<TraceSite>? traceStack = null) : this(messageType, messageCode, null, context, traceStack) {}
        
        [JsonConstructor]
        public CompilerMessage(string messageType, int? messageCode, MessageLevel? messageLevel, string? context, IReadOnlyCollection<TraceSite>? traceStack)
        {
            MessageType = messageType;
            MessageCode = messageCode;
            MessageLevel = (messageCode.HasValue, TryGetMessageLevel(messageType, messageCode.GetValueOrDefault(0), out var level)) switch
            {
                (true, true) => level,
                (true, false) => null,
                (false, _) => messageLevel
            };
            Context = context;
            TraceStack = traceStack?.ToArray() // Force a copy
                         ?? Array.Empty<TraceSite>();
            _message = null;
        }
        private string? _message;

        public string MessageType { get; }
        public int? MessageCode { get; }
        public MessageLevel? MessageLevel { get; }
        public string? Context { get; }
        public IReadOnlyCollection<TraceSite>? TraceStack { get; }

        public override string ToString() => ToString(true);
        
        public string ToString(bool stackTrace)
        {
            if (_message == null)
            {
                var includingStackTrace = stackTrace && TraceStack?.Count > 0;
                if (MessageCode.HasValue || includingStackTrace)
                {
                    var builder = new StringBuilder();
                    if (MessageCode.HasValue)
                    {
                        builder.Append(MessageType).Append(MessageCode).Append(": ").Append(MessageLevel).Append(" - ")
                               .AppendLine(TryGetMessageName(MessageType, MessageCode.Value, out var message)
                                               ? message
                                               : "Unknown");
                    }

                    builder.Append(Context);
                    if (includingStackTrace)
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
                else
                {
                    _message = Context ?? string.Empty;
                }
            }

            return _message;
        }
    }
}