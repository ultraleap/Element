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
    public enum MessageCode
    {
        // GENERAL PURPOSE
        Success = 0,
        FileAccessError = 25,
        ArgumentNotFound = 26,
        ArgumentOutOfRange = 28,
        InvalidCast = 30,
        UnknownError = 9999,
        
        // PARSE
        ParseError= 9,
        DuplicateSourceFile = 27,
        
        // VALIDATION
        MultipleDefinitions = 2,
        IntrinsicNotFound = 4,
        MissingPorts = 13,
        InvalidIdentifier = 15,
        StructCannotHaveReturnType = 19,
        IntrinsicCannotHaveBody = 20,
        MissingFunctionBody = 21,
        MultipleIntrinsicLocations = 29,
        FunctionMissingReturn = 31,
        PortListCannotContainDiscards = 32,
        PortListDeclaresDefaultArgumentBeforeNonDefault = 33,
        IntrinsicConstraintCannotSpecifyFunctionSignature = 34,
        
        // COMPILATION
        SerializationError = 1,
        InvalidCompileTarget = 3,
        LocalShadowing = 5,
        ArgumentCountMismatch = 6,
        IdentifierNotFound = 7,
        ConstraintNotSatisfied = 8,
        InvalidBoundaryFunction = 10,
        RecursionNotAllowed = 11,
        MissingBoundaryConverter = 12,
        TypeError = 14,
        InvalidExpression = 16,
        InvalidReturnType = 17,
        InvalidBoundaryData = 18,
        CannotBeUsedAsInstanceFunction = 22,
        FunctionCannotBeUncurried = 23,
        NotCompileConstant = 24,
        InfiniteLoop = 35,
        NotFunction = 36,
        NotIndexable = 37,
        NotConstraint = 38,
    }
    
    public class CompilerMessage
    {
        private static readonly TomlTable _messageToml = Toml.Parse(File.ReadAllText("Messages.toml")).ToModel();

        private static bool TryGetMessageToml(MessageCode messageCode, out TomlTable? message)
        {
            message = _messageToml.TryGetToml($"ELE{(int)messageCode}", out var obj) && obj is TomlTable table ? table : null;
            return message != null;
        }
        
        public static bool TryGetMessageName(MessageCode messageCode, out string? name)
        {
            name = TryGetMessageToml(messageCode, out var table) ? (string) table!["name"] : null;
            return name != null;
        }

        public static bool TryGetMessageLevel(MessageCode messageCode, out MessageLevel level)
        {
            if (Enum.TryParse(TryGetMessageToml(messageCode, out var table) ? (string) table!["level"] : string.Empty, out level)) return true;
            level = Element.MessageLevel.Error;
            return false;
        }

        public CompilerMessage(string message, MessageLevel? messageLevel = null) : this(null, messageLevel, message, null) {}
        public CompilerMessage(MessageCode messageCode, string? context, IReadOnlyCollection<TraceSite>? traceStack = null) : this((int)messageCode, null, context, traceStack) {}
        
        [JsonConstructor]
        public CompilerMessage(int? messageCode, MessageLevel? messageLevel, string? context, IReadOnlyCollection<TraceSite>? traceStack)
        {
            MessageCode = messageCode;
            MessageLevel = (messageCode.HasValue, TryGetMessageLevel((MessageCode)messageCode.GetValueOrDefault(0), out var level)) switch
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

        public int? MessageCode { get; }
        public MessageLevel? MessageLevel { get; }
        public string? Context { get; }
        public IReadOnlyCollection<TraceSite>? TraceStack { get; }

        public override string ToString()
        {
            if (_message == null)
            {
                if (MessageCode.HasValue || TraceStack?.Count > 0)
                {
                    var builder = new StringBuilder();
                    if (MessageCode.HasValue)
                    {
                        builder.Append("ELE").Append(MessageCode).Append(": ").Append(MessageLevel).Append(" - ")
                               .AppendLine(TryGetMessageName((MessageCode)MessageCode.Value, out var message)
                                               ? message
                                               : "Unknown");
                    }

                    builder.Append(Context);
                    if (TraceStack?.Count > 0)
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