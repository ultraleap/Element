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

                TValue Get<TValue>(string key) => (TValue) tomlTable[key];
                _processHostInfos.Add(new ProcessHostInfo
                {
                    Name = name,
                    Enabled = Get<bool>("enabled"),
                    BuildCommand = Get<string>("build-command"),
                    ExecutablePath = Path.Combine(Laboratory.ElementRootDirectory.Value, Get<string>("executable-path"))
                });
            }
        }

        private class ProcessHostInfo
        {
            public string Name { get; set; }
            public bool Enabled { get; set; }
            public string BuildCommand { get; set; }
            public string ExecutablePath { get; set; }
        }

        private static readonly List<ProcessHostInfo> _processHostInfos = new List<ProcessHostInfo>();

        /// <summary>
        /// Implements commands directly.
        /// </summary>
        private class SelfHost : IHost
        {
            public override string ToString() => "Laboratory";

            float[] IHost.Execute(HostContext context, string functionName, params float[] functionArgs)
            {
                var sourceContext = new SourceContext();
                var compilationContext = new CompilationContext(logToConsole: false);

                static FileInfo[] OpenPackage(string package) => new DirectoryInfo(package).GetFiles("*.ele", SearchOption.AllDirectories);
                if (context.IncludePrelude) sourceContext.AddSourceFiles(OpenPackage("StandardLibrary"));
                sourceContext.AddSourceFiles(context.Packages.SelectMany(OpenPackage));

                var messages = new List<string>();
                var anyErrors = false;

                compilationContext.OnLog += msg => messages.Add(msg);
                compilationContext.OnError += msg =>
                {
                    messages.Add(msg);
                    anyErrors = true;
                };

                sourceContext.Recompile(compilationContext);

                var function = sourceContext.GlobalScope.GetFunction(functionName, compilationContext);
                var result = function.EvaluateAndSerialize(functionArgs, compilationContext);

                context.MessageHandler(messages, anyErrors);

                return result;
            }
        }

        /// <summary>
        /// Implements commands by calling external process defined using a command string.
        /// </summary>
        private class ProcessHost : IHost
        {
            private static (List<string> Messages, bool AnyErrors) Run(Process process)
            {
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.RedirectStandardError = true;
                process.EnableRaisingEvents = true;

                var messages = new List<string>();
                var anyErrors = false;

                process.OutputDataReceived += (_, eventArgs) => messages.Add(eventArgs.Data);
                process.ErrorDataReceived += (_, eventArgs) =>
                {
                    messages.Add(eventArgs.Data);
                    anyErrors = true;
                };

                process.Start();
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();
                process.WaitForExit();

                return (messages, anyErrors);
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

                        var (messages, _) = Run(process);

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

            private List<string> RunHostProcess(HostContext context, string arguments)
            {
                if (_hostBuildErrors.TryGetValue(_info, out var errorMessages))
                {
                    errorMessages.PrintMessagesToTestContext($"{_info.Name} build log");
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

                var (messages, anyErrors) = Run(process);

                if (process.ExitCode != 0)
                {
                    messages.PrintMessagesToTestContext($"{_info.Name} output");
                    Assert.Fail($"{_info.Name} process quit with exit code '{0}'. See output below.", process.ExitCode);
                }

                context.MessageHandler(messages, anyErrors);

                return messages;
            }

            float[] IHost.Execute(HostContext context, string functionName, params float[] functionArgs)
            {
                var processArgs = new StringBuilder();
                processArgs.Append("execute");
                var packages = context.Packages;
                if (context.IncludePrelude || packages?.Length > 0)
                {
                    processArgs.Append(" -p ");
                    if (context.IncludePrelude) processArgs.Append("StandardLibrary ");
                    if (packages?.Length > 0) processArgs.AppendJoin(' ', packages);
                }

                processArgs.Append(" -f ");
                processArgs.Append(functionName);

                if (functionArgs?.Length > 0)
                {
                    processArgs.Append(" -a ");
                    processArgs.AppendJoin(' ', functionArgs);
                }

                return RunHostProcess(context, processArgs?.ToString()).Last().Split(' ', StringSplitOptions.RemoveEmptyEntries).Select(s => float.Parse(s, CultureInfo.InvariantCulture)).ToArray();
            }
        }
    }
}