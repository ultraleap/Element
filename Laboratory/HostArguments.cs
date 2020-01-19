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

            private class CommandInstance
            {
                public CommandInstance(in HostContext hostContext, SourceContext sourceContext = default, CompilationContext compilationContext = default)
                {
                    SourceContext = sourceContext ??= new SourceContext();
                    CompilationContext = compilationContext ??= new CompilationContext(logToConsole: false);
                    HostContext = hostContext;

                    static FileInfo[] OpenPackage(string package) => new DirectoryInfo(package).GetFiles("*.ele", SearchOption.AllDirectories);
                    if (hostContext.IncludePrelude) sourceContext.AddSourceFiles(OpenPackage("Prelude"));
                    sourceContext.AddSourceFiles(hostContext.Packages.SelectMany(OpenPackage));

                    compilationContext.OnLog += msg => Messages.Add(msg);
                    compilationContext.OnError += msg =>
                    {
                        Messages.Add(msg);
                        AnyErrors = true;
                    };

                    sourceContext.Recompile(compilationContext);
                }

                private SourceContext SourceContext { get; }
                private CompilationContext CompilationContext { get; }
                private List<string> Messages { get; } =  new List<string>();
                private bool AnyErrors { get; set; }
                private HostContext HostContext { get; }

                private void PrintAnyErrors() => HostContext.MessageHandler(Messages, AnyErrors);

                public bool Parse(in FileInfo file)
                {
                    SourceContext.AddSourceFiles(new []{file});
                    SourceContext.Recompile(CompilationContext);
                    PrintAnyErrors();
                    return !AnyErrors;
                }

                public float[] Execute(in string functionName, params float[] functionArgs)
                {
                    var function = SourceContext.GlobalScope.GetFunction(functionName, CompilationContext);
                    var result = function.EvaluateAndSerialize(functionArgs, CompilationContext);
                    PrintAnyErrors();
                    return result;
                }
            }

            bool IHost.Parse(HostContext hostContext, FileInfo file) => new CommandInstance(hostContext).Parse(file);

            float[] IHost.Execute(HostContext hostContext, string functionName, params float[] functionArgs) => new CommandInstance(hostContext).Execute(functionName, functionArgs);
        }

        /// <summary>
        /// Implements commands by calling external process defined using a command string.
        /// </summary>
        private readonly struct ProcessHost : IHost
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

            private static StringBuilder BeginCommand(HostContext hostContext, string command)
            {
                var processArgs = new StringBuilder();
                processArgs.Append(command);
                var packages = hostContext.Packages;
                if (hostContext.IncludePrelude || packages?.Length > 0)
                {
                    processArgs.Append(" -p ");
                    if (hostContext.IncludePrelude) processArgs.Append("Prelude ");
                    if (packages?.Length > 0) processArgs.AppendJoin(' ', packages);
                }

                return processArgs;
            }

            bool IHost.Parse(HostContext hostContext, FileInfo file)
            {
                var processArgs = BeginCommand(hostContext, "parse");

                processArgs.Append(" -f ");
                processArgs.Append(file.FullName);

                return bool.Parse(RunHostProcess(hostContext, processArgs.ToString()).Last());
            }

            float[] IHost.Execute(HostContext hostContext, string functionName, params float[] functionArgs)
            {
                var processArgs = BeginCommand(hostContext, "execute");

                processArgs.Append(" -f ");
                processArgs.Append(functionName);

                if (functionArgs?.Length > 0)
                {
                    processArgs.Append(" -a ");
                    processArgs.AppendJoin(' ', functionArgs);
                }

                return RunHostProcess(hostContext, processArgs.ToString()).Last().Split(' ', StringSplitOptions.RemoveEmptyEntries).Select(s => float.Parse(s, CultureInfo.InvariantCulture)).ToArray();
            }
        }
    }
}