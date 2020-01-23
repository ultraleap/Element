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
        public bool ExcludePrelude { get; }
        public IReadOnlyList<DirectoryInfo> Packages { get; }
        public IReadOnlyList<FileInfo> ExtraSourceFiles { get; }
        public Action<CompilerMessage>? LogCallback { get; }

        public CompilationInput(in Action<CompilerMessage>? logCallback,
            in bool excludePrelude = false,
            in IReadOnlyList<DirectoryInfo> packages = null,
            in IReadOnlyList<FileInfo> extraSourceFiles = null,
            in string compilerFlagsToml = null)
        {
            LogCallback = logCallback;
            ExcludePrelude = excludePrelude;
            Packages = packages ?? Array.Empty<DirectoryInfo>();
            ExtraSourceFiles = extraSourceFiles ?? Array.Empty<FileInfo>();
            _compilerFlags = Toml.Parse(compilerFlagsToml ?? File.ReadAllText("CompilerFlags.toml")).ToModel();
        }
        
        private readonly TomlTable _compilerFlags;

        private TValue CompilerFlag<TValue>(TValue defaultValue, [CallerMemberName] string caller = default) =>
            ((TomlTable) _compilerFlags[caller ?? throw new ArgumentNullException(nameof(caller))])
                .TryGetValue("value", out var value) switch
                {
                    true => typeof(TValue) switch
                    {
                        { } type when type.IsEnum => (TValue)Enum.Parse(type, (string)value),
                        _ => (TValue)value
                    },
                    false => defaultValue
                };

        public bool Debug => CompilerFlag(false);
        public MessageLevel Verbosity => CompilerFlag(MessageLevel.Information);
    }
}