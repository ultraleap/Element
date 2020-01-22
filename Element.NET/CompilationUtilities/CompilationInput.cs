using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using Tomlyn;
using Tomlyn.Model;

namespace Element
{
    /// <summary>
    /// Contain input data for compiler including configuration
    /// </summary>
    public readonly struct CompilationInput
    {
        public bool LogToConsole { get; }
        public bool ExcludePrelude { get; }
        public IReadOnlyList<DirectoryInfo> Packages { get; }

        public CompilationInput(in bool logToConsole, in bool excludePrelude, in List<DirectoryInfo> packages, in string compilerFlagsToml = null)
        {
            LogToConsole = logToConsole;
            ExcludePrelude = excludePrelude;
            Packages = packages ?? new List<DirectoryInfo>(0);
            CompilerFlags = Toml.Parse(compilerFlagsToml ?? File.ReadAllText("CompilerFlags.toml")).ToModel();
        }
        
        private readonly TomlTable CompilerFlags;

        private TValue CompilerFlag<TValue>(TValue defaultValue, [CallerMemberName] string caller = default) =>
            ((TomlTable) CompilerFlags[caller ?? throw new ArgumentNullException(nameof(caller))])
                .TryGetValue("value", out var value) switch
                {
                    true => typeof(TValue) switch
                    {
                        { } enumType when enumType.IsEnum => Enum.TryParse(enumType, (string)value, out var enumValue) ? enumValue : defaultValue,
                        _ => (TValue)value
                    },
                    false => defaultValue
                };

        public bool Debug => CompilerFlag(false);
        public MessageLevel Verbosity => CompilerFlag(MessageLevel.Information);
    }
}