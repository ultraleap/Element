using System.Collections.Generic;
using System.Text;

namespace Element
{
    public readonly struct CompilerMessage
    {
        public CompilerMessage(in int? messageCode, in string? name, in MessageLevel? messageLevel, in string context, in CallSite[] callStack)
        {
            MessageCode = messageCode;
            Name = name;
            Level = messageLevel;
            CallStack = callStack;

            var builder = new StringBuilder();
            if (messageCode.HasValue)
            {
                builder.Append("ELE").Append(messageCode.Value).Append(": ").Append(Level).Append(" - ")
                    .Append(Name).AppendLine();
            }

            builder.AppendLine(context);
            builder.AppendLine(callStack.Length > 0 ? "Stack trace:" : "No stack trace");
            if (callStack.Length > 0)
            {
                foreach (var frame in CallStack)
                {
                    builder.Append("    ").AppendLine(frame.ToString());
                }
            }

            _message = builder.ToString();
        }
        private readonly string _message;

        public int? MessageCode { get; }
        public string? Name { get; }
        public MessageLevel? Level { get; }
        public IReadOnlyCollection<CallSite> CallStack { get; }

        public override string ToString() => _message;
    }
}