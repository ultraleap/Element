using System;
using System.Collections.Generic;
using System.Text;
using Newtonsoft.Json;

namespace Element
{
    public readonly struct CompilerMessage
    {
        public CompilerMessage(string message, DateTime? timeStamp = default, MessageLevel? messageLevel = default) : this()
        {
            _message = Context = message;
            MessageLevel = messageLevel;
            TimeStamp = timeStamp ?? DateTime.Now;
        }

        [JsonConstructor]
        public CompilerMessage(int? messageCode, string? name, MessageLevel? messageLevel, string? context, DateTime timeStamp, IReadOnlyList<TraceSite> callStack)
        {
            MessageCode = messageCode;
            Name = name;
            Context = context;
            MessageLevel = messageLevel;
            TimeStamp = timeStamp;
            CallStack = callStack;

            var builder = new StringBuilder();
            if (messageCode.HasValue)
            {
                builder.Append("ELE").Append(messageCode.Value).Append(": ").Append(MessageLevel).Append(" - ")
                    .Append(Name).AppendLine();
            }

            builder.Append(context);
            if (messageCode.HasValue && callStack?.Count > 0)
            {
                builder.AppendLine();
                builder.AppendLine("Element source trace:");
                foreach (var site in CallStack)
                {
                    builder.Append("    ").AppendLine(site.ToString());
                }
                builder.AppendLine();
            }

            _message = builder.ToString();
        }
        private readonly string _message;

        public string? Context { get; }
        public int? MessageCode { get; }
        public string? Name { get; }
        public MessageLevel? MessageLevel { get; }
        public IReadOnlyCollection<TraceSite> CallStack { get; }
        public DateTime TimeStamp { get; }

        public override string ToString() => _message;
    }
}