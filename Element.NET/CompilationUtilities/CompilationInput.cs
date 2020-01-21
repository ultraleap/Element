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
        public Action<string>? MessageHandler { get; }
        public Action<string>? ErrorHandler { get; }

        public CompilationInput(bool logToConsole, bool excludePrelude,
                                List<DirectoryInfo> packages, Action<string>? messageHandler,
                                Action<string>? errorHandler)
        {
            LogToConsole = logToConsole;
            ExcludePrelude = excludePrelude;
            Packages = packages ?? new List<DirectoryInfo>(0);
            MessageHandler = messageHandler;
            ErrorHandler = errorHandler;
        }
        
        private static readonly TomlTable CompilerFlags = Toml.Parse(File.ReadAllText("CompilerFlags.toml")).ToModel();
        private static TValue CompilerFlag<TValue>([CallerMemberName] string caller = default) => (TValue)((TomlTable)CompilerFlags[caller ?? throw new ArgumentNullException(nameof(caller))])["value"];

        public bool Debug => CompilerFlag<bool>();
        public MessageLevel Verbosity => (MessageLevel)Enum.Parse(typeof(MessageLevel), CompilerFlag<string>());
    }
}