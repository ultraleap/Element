using System;
using System.Collections.Generic;
using System.Text;

namespace Element
{
    public class CompilerMessage
    {
        public CompilerMessage(int? messageCode, string? name, MessageLevel? messageLevel, string context, Stack<CallSite> callStack, bool appendStackTrace)
        {
            Name = name;
            Level = messageLevel;
            CallStack = callStack.Clone();
            _message = new Lazy<string>(() =>
            {
                var builder = new StringBuilder();
                if (messageCode.HasValue)
                {
                    builder.Append("ELE").Append(messageCode.Value).Append(": ").Append(Level).Append(" - ")
                        .Append(Name).AppendLine();
                }

                builder.AppendLine(context);
                builder.AppendLine();
                if (appendStackTrace)
                {
                    builder.AppendLine("Stack trace:");
                    foreach (var frame in CallStack)
                    {
                        builder.Append("    ").AppendLine(frame.ToString());
                    }
                }
                return builder.ToString();
            });
        }
        private readonly Lazy<string> _message;

        public string? Name { get; }
        public MessageLevel? Level { get; }
        public Stack<CallSite> CallStack { get; }

        public override string ToString() => _message.Value;
    }
}