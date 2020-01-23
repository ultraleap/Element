using System.Collections.Generic;
using System.Text;

namespace Element
{
    public struct CompilerMessage
    {
        public CompilerMessage(in string message, in MessageLevel? messageLevel = default) : this()
        {
            _message = Context = message;
            Level = messageLevel;
        }

        public CompilerMessage(in int? messageCode, in string? name, in MessageLevel? messageLevel, in string? context, in CallSite[] callStack)
        {
            MessageCode = messageCode;
            Name = name;
            Context = context;
            Level = messageLevel;
            CallStack = callStack;

            var builder = new StringBuilder();
            if (messageCode.HasValue)
            {
                builder.Append("ELE").Append(messageCode.Value).Append(": ").Append(Level).Append(" - ")
                    .Append(Name).AppendLine();
            }

            builder.Append(context);
            if (messageCode.HasValue)
            {
                builder.AppendLine();
                builder.AppendLine(callStack?.Length > 0 ? "Stack trace:" : "No stack trace");
                if (callStack?.Length > 0)
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

        public string Context { get; set; }
        public int? MessageCode { get; set; }
        public string? Name { get; set; }
        public MessageLevel? Level { get; set; }
        public IReadOnlyCollection<CallSite> CallStack { get; set; }

        public override string ToString() => _message;
    }
}