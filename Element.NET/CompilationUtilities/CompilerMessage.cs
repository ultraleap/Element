using System;
using System.Collections.Generic;
using System.Text;
using Newtonsoft.Json;

namespace Element
{
    public readonly struct CompilerMessage
    {
        public CompilerMessage(in string message, in DateTime? timeStamp = default, in MessageLevel? messageLevel = default) : this()
        {
            _message = Context = message;
            MessageLevel = messageLevel;
            TimeStamp = timeStamp ?? DateTime.Now;
        }

        [JsonConstructor]
        public CompilerMessage(in int? messageCode, in string? name, in MessageLevel? messageLevel, in string? context, in DateTime timeStamp, in IReadOnlyList<CallSite> callStack)
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
            if (messageCode.HasValue)
            {
                builder.AppendLine();
                builder.AppendLine(callStack?.Count > 0 ? "Stack trace:" : "No stack trace");
                if (callStack?.Count > 0)
                {
                    foreach (var frame in CallStack)
                    {
                        builder.Append("    ").AppendLine(frame.ToString());
                    }
                }
            }

            _message = builder.ToString();
        }
        private readonly string _message;

        public string? Context { get; }
        public int? MessageCode { get; }
        public string? Name { get; }
        public MessageLevel? MessageLevel { get; }
        public IReadOnlyCollection<CallSite> CallStack { get; }
        public DateTime TimeStamp { get; }

        public override string ToString() => _message;
    }
}