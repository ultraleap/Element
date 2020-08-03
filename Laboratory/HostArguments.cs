using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Element;
using Newtonsoft.Json;
using NUnit.Framework;

namespace Laboratory
{
    /// <summary>
    /// Implements hosts types and enumerates hosts for host fixtures
    /// </summary>
    internal static class HostArguments
    {
        public static IHost MakeHost() => _processHostInfo != null
                                            ? (IHost)new ProcessHost(_processHostInfo)
                                            : new AtomicHost();
        
        static HostArguments()
        {
            static TValue Get<TValue>(string key) => TestContext.Parameters.Get<TValue>(key, default);
            var name = Get<string>("name");
            
            _processHostInfo = string.IsNullOrEmpty(name)
                                   ? (ProcessHostInfo?)null
                                   : new ProcessHostInfo(name, 
                                       new []{Get<string>("build-command") ,Get<string>("additional-build-command")},
                                       Path.Combine(_elementRootDirectory.Value, Get<string>("executable-path")), 
                                       Get<string>("working-directory"));
        }

        private static readonly Lazy<string> _elementRootDirectory = new Lazy<string>(() =>
        {
            var success = TryGetParent(new DirectoryInfo(Directory.GetCurrentDirectory()), "Element", out var rootDir);
            if (!success) throw new DirectoryNotFoundException("Couldn't find Element root directory");
            return rootDir.FullName;
        });

        private static bool TryGetParent(DirectoryInfo directory, string parentName, out DirectoryInfo parent)
        {
            parent = null;
            while (true)
            {
                if (directory.Parent == null) return false;
                if (string.Equals(directory.Parent.Name, parentName, StringComparison.OrdinalIgnoreCase))
                {
                    parent = directory.Parent;
                    return true;
                }

                directory = directory.Parent;
            }
        }

        private class ProcessHostInfo
        {
            public ProcessHostInfo(string name, string[] buildCommands, string executablePath, string workingDirectory)
            {
                Name = name;
                BuildCommands = buildCommands;
                ExecutablePath = executablePath;
                WorkingDirectory = workingDirectory;
            }

            public string Name { get; }
            public string[] BuildCommands { get; }
            public string ExecutablePath { get; }
            public string WorkingDirectory { get; }
        }

        private static readonly ProcessHostInfo? _processHostInfo;
        static async Task ReadStream(List<string> messages, Process proc, StreamReader streamReader)
        {
            while (!proc.HasExited || !streamReader.EndOfStream)
            {
                var msg = await streamReader.ReadLineAsync();
                if (!string.IsNullOrEmpty(msg))
                    messages.Add(msg);
            }
        }

        /// <summary>
        /// Implements commands by calling external process defined using a command string.
        /// </summary>
        private readonly struct ProcessHost : IHost
        {
            private static List<string> Run(Process process)
            {
                var messages = new List<string>();

                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.RedirectStandardError = true;
                process.Start();
                var readingStdOut = ReadStream(messages, process, process.StandardOutput);
                var readingStdErr = ReadStream(messages, process, process.StandardError);
                process.WaitForExit();
                Task.WhenAll(readingStdOut, readingStdErr).Wait();
                return messages;
            }

            static ProcessHost()
            {
                if (_processHostInfo == null) return;

                // Perform build command - within static constructor so it's only performed once per test run
                var messages = new List<string>();
                try
                {
                    foreach(var command in _processHostInfo.BuildCommands.Where(s => !string.IsNullOrWhiteSpace(s)))
                    {
                        var splitCommand = command.Split(' ', 2);
                        var process = new Process
                        {
                            StartInfo = new ProcessStartInfo
                            {
                                FileName = splitCommand[0],
                                Arguments = splitCommand[1],
                                WorkingDirectory = _elementRootDirectory.Value
                            }
                        };

                        process.StartInfo.RedirectStandardError = true;
                        process.StartInfo.RedirectStandardOutput = true;
                        process.Start();
                        var readingStdOut = ReadStream(messages, process, process.StandardOutput);
                        var readingStdErr = ReadStream(messages, process, process.StandardError);
                        process.WaitForExit();

                        if (process.ExitCode != 0)
                        {
                            Task.WhenAll(readingStdOut, readingStdErr).Wait();
                            _hostBuildErrors.Add(_processHostInfo, messages);
                        }
                    }
                }
                catch (Exception e)
                {
                    messages.Add(e.ToString());
                    _hostBuildErrors.Add(_processHostInfo, messages);
                }
            }

            private static bool TryParseJson<T>(string json, out T result)
            {
                var success = true;
                var settings = new JsonSerializerSettings
                {
                    Error = (sender, args) => { success = false; args.ErrorContext.Handled = true; },
                    MissingMemberHandling = MissingMemberHandling.Error
                };

                try
                {
                    result = JsonConvert.DeserializeObject<T>(json, settings);
                }
                catch(Exception e)
                {
                    result = default;
                    success = false;
                }

                return success;
            }

            private static readonly Dictionary<ProcessHostInfo, List<string>> _hostBuildErrors = new Dictionary<ProcessHostInfo, List<string>>();

            public ProcessHost(ProcessHostInfo info) => _info = info;

            public override string ToString() => _info.Name;

            private readonly ProcessHostInfo _info;

            private Result<string> RunHostProcess(Context context, string arguments)
            {
                if (_hostBuildErrors.TryGetValue(_info, out var buildError))
                {
                    Assert.Fail(string.Join("\n", buildError));
                }

                var process = new Process
                {
                    StartInfo = new ProcessStartInfo
                    {
                        FileName = _info.ExecutablePath,
                        Arguments = arguments,
                        WorkingDirectory = Path.Combine(_elementRootDirectory.Value, _info.WorkingDirectory)
                    }
                };

                var messages = Run(process);

                var resultBuilder = new ResultBuilder<string>(context, string.Empty);

                foreach (var msg in messages)
                {
                    resultBuilder.Append(TryParseJson(msg, out CompilerMessage compilerMessage)
                                             ? compilerMessage
                                             : new CompilerMessage(msg));
                }

                if (process.ExitCode != 0 && !resultBuilder.Messages.Any(m => m.MessageLevel.HasValue && m.MessageLevel.Value >= MessageLevel.Error))
                {
                    resultBuilder.Append(MessageCode.UnknownError, $"{_info.Name} process quit with exit code '{process.ExitCode}'.");
                }

                process.Close();
                resultBuilder.Result = resultBuilder.Messages.Last().ToString();

                return resultBuilder.ToResult();
            }

            private static StringBuilder BeginCommand(CompilerInput input, string command)
            {
                var processArgs = new StringBuilder();
                processArgs.Append($"{command} --logjson");
                if (input.Source.ExcludePrelude) processArgs.Append(" --no-prelude ");
                if (input.Source.Packages.Count > 0) processArgs.Append(" --packages ").AppendJoin(' ', input.Source.Packages);
                if (input.Source.ExtraSourceFiles.Count > 0) processArgs.Append(" --source-files ").AppendJoin(' ', input.Source.ExtraSourceFiles);
                if (!input.Options.ReleaseMode) processArgs.Append(" --debug ");
                if (input.Options.SkipValidation) processArgs.Append(" --no-validation ");
                if (input.Options.NoParseTrace) processArgs.Append(" --no-parse-trace ");
                return processArgs;
            }

            Result IHost.Parse(CompilerInput input) => (Result)RunHostProcess(new Context(null, input.Options), BeginCommand(input, "parse").ToString());

            Result<float[]> IHost.Evaluate(CompilerInput input, string expression)
            {
                var resultBuilder = new ResultBuilder<float[]>(new Context(null, input.Options), Array.Empty<float>());
                return RunHostProcess(resultBuilder.Context, BeginCommand(input, "evaluate").Append($" -e \"{expression}\"").ToString())
                    .Bind(resultString =>
                    {
                        resultBuilder.Result = resultString.Split(' ', StringSplitOptions.RemoveEmptyEntries)
                                                           .Select(s =>
                                                           {
                                                               if (!float.TryParse(s, NumberStyles.Float, CultureInfo.InvariantCulture, out var value))
                                                               {
                                                                   resultBuilder.Append(MessageCode.ParseError, $"Could not parse result string '{s}' as a float");
                                                               }
                                                               return value;
                                                           })
                                                           .ToArray();
                        return resultBuilder.ToResult();
                    });
            }

            Result<string> IHost.Typeof(CompilerInput input, string expression) =>
                RunHostProcess(new Context(null, input.Options), BeginCommand(input, "typeof").Append($" -e \"{expression}\"").ToString());
        }
    }
}