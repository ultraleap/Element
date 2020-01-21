using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using Element;
using Element.CLR;
using NUnit.Framework;
using Tomlyn;
using Tomlyn.Model;

namespace Laboratory
{
    /// <summary>
    /// Implements hosts types and enumerates hosts for host fixtures
    /// </summary>
    internal class HostArguments : IEnumerable
    {
        public IEnumerator GetEnumerator() => _processHostInfos
            .Where(phi => phi.Enabled)
            .Select(phi => (IHost) new ProcessHost(phi))
            .Prepend(new SelfHost())
            .ToArray<object>()
            .GetEnumerator();

        static HostArguments()
        {
            var processHosts = Toml.Parse(File.ReadAllText("ProcessHostConfigurations.toml")).ToModel();
            foreach ((string name, object table) in processHosts)
            {
                var tomlTable = (TomlTable) table;

                TValue Get<TValue>(in string key) => (TValue) tomlTable[key];
                _processHostInfos.Add(new ProcessHostInfo
                (
                    name,
                    Get<bool>("enabled"),
                    Get<string>("build-command"),
                    Path.Combine(Laboratory.ElementRootDirectory.Value, Get<string>("executable-path"))
                ));
            }
        }

        private readonly struct ProcessHostInfo
        {
            public ProcessHostInfo(in string name, bool enabled, in string buildCommand, in string executablePath)
            {
                Name = name;
                Enabled = enabled;
                BuildCommand = buildCommand;
                ExecutablePath = executablePath;
            }

            public string Name { get; }
            public bool Enabled { get; }
            public string BuildCommand { get; }
            public string ExecutablePath { get; }
        }

        private static readonly List<ProcessHostInfo> _processHostInfos = new List<ProcessHostInfo>();

        /// <summary>
        /// Implements commands directly.
        /// </summary>
        private readonly struct SelfHost : IHost
        {
            public override string ToString() => "Laboratory";

            bool IHost.ParseFiles(in CompilationInput compilationInput, in IEnumerable<FileInfo> files) => new HostCommand(compilationInput).Parse(files);

            float[] IHost.Execute(in CompilationInput compilationInput, in string functionName, params float[] functionArgs) => new HostCommand(compilationInput).Execute(functionName, functionArgs);
        }

        /// <summary>
        /// Implements commands by calling external process defined using a command string.
        /// </summary>
        private readonly struct ProcessHost : IHost
        {
            private static void Run(Process process, Action<string> onMessage, Action<string> onError)
            {
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.RedirectStandardError = true;
                process.EnableRaisingEvents = true;

                process.OutputDataReceived += (_, eventArgs) => onMessage?.Invoke(eventArgs.Data);
                process.ErrorDataReceived += (_, eventArgs) => onError?.Invoke(eventArgs.Data);

                process.Start();
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();
                process.WaitForExit();
            }

            static ProcessHost()
            {
                // Perform build command for each enabled host - within static constructor so it's only performed once per test run
                foreach (var info in _processHostInfos.Where(info => info.Enabled))
                {
                    try
                    {
                        var splitCommand = info.BuildCommand.Split(' ', 2);
                        var process = new Process
                        {
                            StartInfo = new ProcessStartInfo
                            {
                                FileName = splitCommand[0],
                                Arguments = splitCommand[1],
                                WorkingDirectory = Laboratory.ElementRootDirectory.Value
                            }
                        };

                        var messages = new List<string>();
                        void CacheMessage(string msg) => messages.Add(msg);
                        Run(process, CacheMessage, CacheMessage);

                        if (process.ExitCode != 0)
                        {
                            _hostBuildErrors.Add(info, messages);
                        }
                    }
                    catch (Exception e)
                    {
                        _hostBuildErrors.Add(info, new List<string>{e.ToString()});
                    }
                }
            }

            private static readonly Dictionary<ProcessHostInfo, List<string>> _hostBuildErrors = new Dictionary<ProcessHostInfo, List<string>>();

            public ProcessHost(ProcessHostInfo info) => _info = info;

            public override string ToString() => _info.Name;

            private readonly ProcessHostInfo _info;

            private string RunHostProcess(CompilationInput input, string arguments)
            {
                if (_hostBuildErrors.TryGetValue(_info, out var messages))
                {
                    messages.PrintMessagesToTestContext($"{_info.Name} build log");
                    Assert.Fail($"{_info.Name} failed to build. See build log below.");
                }

                var process = new Process
                {
                    StartInfo = new ProcessStartInfo
                    {
                        FileName = _info.ExecutablePath,
                        Arguments = arguments
                    }
                };

                var result = string.Empty;
                Run(process, msg =>
                {
                    input.MessageHandler?.Invoke(msg);
                    result = msg;
                }, input.ErrorHandler);

                if (process.ExitCode != 0)
                {
                    Assert.Fail($"{_info.Name} process quit with exit code '{process.ExitCode}'.");
                }

                return result;
            }

            private static StringBuilder BeginCommand(CompilationInput input, string command)
            {
                var processArgs = new StringBuilder();
                processArgs.Append(command);
                var packages = input.Packages;
                if (!input.ExcludePrelude || packages.Count > 0)
                {
                    processArgs.Append(" -p ");
                    if (!input.ExcludePrelude) processArgs.Append("Prelude ");
                    if (packages.Count > 0) processArgs.AppendJoin(' ', packages);
                }

                return processArgs;
            }

            bool IHost.ParseFiles(in CompilationInput input, in IEnumerable<FileInfo> files)
            {
                var processArgs = BeginCommand(input, "parse");

                processArgs.Append(" -f ");
                processArgs.AppendJoin(' ', files);

                return bool.Parse(RunHostProcess(input, processArgs.ToString()));
            }

            float[] IHost.Execute(in CompilationInput input, in string functionName, params float[] functionArgs)
            {
                var processArgs = BeginCommand(input, "execute");

                processArgs.Append(" -f ");
                processArgs.Append(functionName);

                if (functionArgs?.Length > 0)
                {
                    processArgs.Append(" -a ");
                    processArgs.AppendJoin(' ', functionArgs);
                }

                return RunHostProcess(input, processArgs.ToString()).Split(' ', StringSplitOptions.RemoveEmptyEntries).Select(s => float.Parse(s, CultureInfo.InvariantCulture)).ToArray();
            }
        }
    }
}