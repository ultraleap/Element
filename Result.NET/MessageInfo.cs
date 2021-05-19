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
    
    public record MessageInfo(string TypePrefix, string? Name, MessageLevel Level, int? Code)
    {
        public static Dictionary<string, Func<int, MessageInfo>> GetFuncsByPrefix { get; } = new();

        public static MessageInfo GetByPrefixAndCode(string prefix, int code)
            => GetFuncsByPrefix.TryGetValue(prefix, out var func)
                ? func(code)
                : throw new InvalidOperationException($"No message info found for '{prefix}{code}' - prefix is missing get function");

        public string NameOrUnknown => (string.IsNullOrEmpty(Name)
            ? "Unknown"
            : Name)!;

        public override string ToString() =>
            string.IsNullOrEmpty(TypePrefix) && string.IsNullOrEmpty(Code.ToString()) // Avoid the leading space if there's no prefix/code
                ? NameOrUnknown
                : $"{TypePrefix}{Code} {NameOrUnknown}";

        public static MessageInfo CustomError(string message, string? typePrefix = null, string? messageName = null) =>
            new(typePrefix ?? string.Empty, messageName, MessageLevel.Error, null);
        
        public static MessageInfo CustomWarning(string message, string? typePrefix = null, string? messageName = null) =>
            new(typePrefix ?? string.Empty, messageName, MessageLevel.Warning, null);
        
        public static MessageInfo CustomInfo(string message, string? typePrefix = null, string? messageName = null) =>
            new(typePrefix ?? string.Empty, messageName, MessageLevel.Information, null);

        public static MessageInfo CustomVerbose(string message, string? typePrefix = null, string? messageName = null) =>
            new(typePrefix ?? string.Empty, messageName, MessageLevel.Verbose, null);
    }

    
}