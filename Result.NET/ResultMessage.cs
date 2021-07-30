using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Newtonsoft.Json;

namespace ResultNET
{
    public class ResultMessage
    {
        public ResultMessage(string message, string messageTypePrefix = "", MessageLevel? messageLevel = null, IReadOnlyCollection<TraceSite>? traceStack = null)
            : this(new MessageInfo(messageTypePrefix, null, messageLevel.GetValueOrDefault(), null), message, traceStack) {}
        public ResultMessage(string messageTypePrefix, int messageCode, string? context, IReadOnlyCollection<TraceSite>? traceStack = null)
            : this(MessageInfo.GetByPrefixAndCode(messageTypePrefix, messageCode), context, traceStack) {}
        
        [JsonConstructor]
        public ResultMessage(string messageTypePrefix, int? messageCode, string? messageName, MessageLevel? messageLevel, string? context, IReadOnlyCollection<TraceSite>? traceStack)
            : this(messageCode.HasValue
                    // Get message by prefix/code if there's a code
                    ? MessageInfo.GetByPrefixAndCode(messageTypePrefix, messageCode.Value)
                    // Else we make a message info out of what was passed
                    : new MessageInfo(messageTypePrefix, messageName, messageLevel.GetValueOrDefault(), messageCode),
                context, traceStack) {}

        public ResultMessage(MessageInfo messageInfo, string? context, IReadOnlyCollection<TraceSite>? traceStack)
        {
            Info = messageInfo;
            Context = context;
            TraceStack = traceStack?.ToArray() // Force a copy
                ?? Array.Empty<TraceSite>();
            _message = null;
        }
        private string? _message;

        public readonly MessageInfo Info;
        public string? Context { get; }
        public IReadOnlyCollection<TraceSite>? TraceStack { get; }

        public override string ToString() => ToString(true);
        
        public string ToString(bool stackTrace)
        {
            if (_message == null)
            {
                var includingStackTrace = stackTrace && TraceStack?.Count > 0;
                if (Info.Code.HasValue || includingStackTrace)
                {
                    var builder = new StringBuilder();
                    if (Info.Code.HasValue)
                    {
                        builder.Append(Info.TypePrefix)
                               .Append(Info.Code)
                               .Append(": ")
                               .Append(Info.Level)
                               .Append(" - ")
                               .AppendLine(Info.NameOrUnknown);
                    }

                    builder.Append(Context);
                    if (includingStackTrace)
                    {
                        builder.AppendLine();
                        builder.AppendLine("Element source trace:");
                        foreach (var site in TraceStack!)
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