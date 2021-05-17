using System;
using System.Collections.Generic;

namespace ResultNET
{
    public enum MessageLevel
    {
        Verbose = -1,
        Information = 0, // Information is the default level
        Warning,
        Error,
    }
    
    public readonly struct MessageInfo
    {
        public static Dictionary<string, Func<int, MessageInfo>> GetFuncsByPrefix { get; } = new Dictionary<string, Func<int, MessageInfo>>();

        public static MessageInfo GetByPrefixAndCode(string prefix, int code)
            => GetFuncsByPrefix.TryGetValue(prefix, out var func)
                ? func(code)
                : throw new InvalidOperationException($"No message info found for '{prefix}{code}' - prefix is missing get function");
        
        public string TypePrefix { get; }
        public string? Name { get; }

        public string NameOrUnknown => (string.IsNullOrEmpty(Name)
            ? "Unknown"
            : Name)!;
        public MessageLevel Level { get; }
        public int? Code { get; }

        public MessageInfo(string typePrefix, string? name, MessageLevel level, int? code)
        {
            TypePrefix = typePrefix;
            Name = name;
            Level = level;
            Code = code;
        }

        public static MessageInfo CustomError(string message, string? typePrefix = null, string? messageName = null) =>
            new MessageInfo(typePrefix ?? string.Empty, messageName, MessageLevel.Error, null);
        
        public static MessageInfo CustomWarning(string message, string? typePrefix = null, string? messageName = null) =>
            new MessageInfo(typePrefix ?? string.Empty, messageName, MessageLevel.Warning, null);
        
        public static MessageInfo CustomInfo(string message, string? typePrefix = null, string? messageName = null) =>
            new MessageInfo(typePrefix ?? string.Empty, messageName, MessageLevel.Information, null);

        public static MessageInfo CustomVerbose(string message, string? typePrefix = null, string? messageName = null) =>
            new MessageInfo(typePrefix ?? string.Empty, messageName, MessageLevel.Verbose, null);
    }

    
}