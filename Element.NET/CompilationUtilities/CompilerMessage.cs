using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using Newtonsoft.Json;
using Tomlyn;
using Tomlyn.Model;

namespace Element
{
    public enum MessageCode
    {
        Success = 0,
        SerializationError = 1,
        MultipleDefinitions = 2,
        InvalidCompileTarget = 3,
        IntrinsicNotFound = 4,
        LocalShadowing = 5,
        ArgumentCountMismatch = 6,
        IdentifierNotFound = 7,
        ConstraintNotSatisfied = 8,
        ParseError= 9,
        InvalidBoundaryFunction = 10,
        CircularCompilation = 11,
        MissingBoundaryConverter = 12,
        MissingPorts = 13,
        TypeError = 14,
        InvalidIdentifier = 15,
        InvalidExpression = 16,
        InvalidReturnType = 17,
        InvalidBoundaryData = 18,
        StructCannotHaveReturnType = 19,
        IntrinsicCannotHaveBody = 20,
        MissingFunctionBody = 21,
        CannotBeUsedAsInstanceFunction = 22,
        FunctionCannotBeUncurried = 23,
        NotCompileConstant = 24,
        FileAccessError = 25,
        ArgumentNotFound = 26,
        DuplicateSourceFile = 27,
        ArgumentOutOfRange = 28,
        MultipleIntrinsicLocations = 29,
        InvalidCast = 30,
        UnknownError = 9999,
    }
    
    public class CompilerMessage
    {
        private static readonly TomlTable _messageToml = Toml.Parse(File.ReadAllText("Messages.toml")).ToModel();

        private static bool TryGetMessageToml(int messageCode, out TomlTable message)
        {
            message = _messageToml.TryGetToml($"ELE{messageCode}", out var obj) && obj is TomlTable table ? table : null;
            return message != null;
        }
        
        public static bool TryGetMessageName(int messageCode, out string name)
        {
            name = TryGetMessageToml(messageCode, out var table) ? (string) table["name"] : null;
            return name != null;
        }

        public static bool TryGetMessageLevel(int messageCode, out MessageLevel level) =>
            Enum.TryParse(TryGetMessageToml(messageCode, out var table) ? (string) table["level"] : null, out level);

        public CompilerMessage(string message, MessageLevel? messageLevel = null) : this(null, messageLevel, message, null) {}
        public CompilerMessage(MessageCode messageCode, string? context, IReadOnlyCollection<TraceSite>? traceStack = null) : this((int)messageCode, null, context, traceStack) {}
        
        [JsonConstructor]
        public CompilerMessage(int? messageCode, MessageLevel? messageLevel, string? context, IReadOnlyCollection<TraceSite>? traceStack)
        {
            MessageCode = messageCode;
            MessageLevel = messageCode.HasValue && TryGetMessageLevel(messageCode.Value, out var level)
                               ? level
                               : messageLevel ?? Element.MessageLevel.Information;
            Context = context;
            TraceStack = traceStack ?? Array.Empty<TraceSite>();
            _message = null;
        }
        private string? _message;

        public int? MessageCode { get; }
        public MessageLevel? MessageLevel { get; }
        public string? Context { get; }
        public IReadOnlyCollection<TraceSite> TraceStack { get; }

        public override string ToString()
        {
            if (_message == null)
            {
                if (MessageCode.HasValue || TraceStack.Count > 0)
                {
                    var builder = new StringBuilder();
                    if (MessageCode.HasValue)
                    {
                        builder.Append("ELE").Append(MessageCode).Append(": ").Append(MessageLevel).Append(" - ")
                               .AppendLine(TryGetMessageName(MessageCode.Value, out var message)
                                               ? message
                                               : "Unknown");
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
                else
                {
                    _message = Context ?? string.Empty;
                }
            }

            return _message;
        }
    }
}