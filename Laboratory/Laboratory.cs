using Element.CLR;

namespace Element.Laboratory
{
	using System.Collections;
	using System.Globalization;
	using System.Text;
	using System;
	using System.Collections.Generic;
	using System.Linq;
	using System.Diagnostics;
	using System.IO;
	using NUnit.Framework;

	internal static class Laboratory
	{
		internal static void Check(this List<string> messages, string messageCode)
		{
			if (string.IsNullOrEmpty(messageCode)) throw new ArgumentNullException(nameof(messageCode));
			Assert.True(messages.Any(m => m.StartsWith(messageCode)));
		}

		internal const float FloatEpsilon = 1.19209e-7f;

		private static void PrintMessagesToTestContext(this List<string> messages)
		{
			TestContext.WriteLine("============================");
			TestContext.WriteLine("Beginning of compiler output");
			TestContext.WriteLine("============================");
			foreach (var message in messages)
			{
				TestContext.WriteLine(message);
			}

			TestContext.WriteLine("======================");
			TestContext.WriteLine("End of compiler output");
			TestContext.WriteLine("======================");
		}
		
		internal interface ICommandInterface
		{
			CommandContext Context { get; }
			float[] Execute(string functionName, params float[] functionArgs);
		}
		
		internal delegate void MessageHandler(List<string> messages, bool anyErrors);
			
		internal class CommandContext
		{
			public bool IncludePrelude { get; set; }
			public string[] Packages { get; } = Array.Empty<string>();
			public MessageHandler MessageHandler { get; set; } = (messages, anyErrors) =>
			{
				messages.PrintMessagesToTestContext();
				if (anyErrors)
				{
					Assert.Fail("Unexpected error - see below for error received.");
				}
			};
		}

		[TestFixtureSource(typeof(FixtureArguments)), Parallelizable(ParallelScope.All)]
		internal abstract class CompilerFixture : ICommandInterface
		{
			static CompilerFixture()
			{
				var process = new Process
				{
					StartInfo = new ProcessStartInfo
					{
						FileName = "BuildCompilers.bat",
						UseShellExecute = true,
						WorkingDirectory = "../../../"
					}
				};
				process.Start();
				process.WaitForExit();
			}
			
			protected CompilerFixture(ICommandInterface commandInterface)
			{
				_commandInterfaceImplementation = commandInterface;
			}

			private readonly ICommandInterface _commandInterfaceImplementation;

			public CommandContext Context => _commandInterfaceImplementation.Context;

			public float[] Execute(string functionName, params float[] functionArgs) =>
				_commandInterfaceImplementation.Execute(functionName, functionArgs);

			private class FixtureArguments : IEnumerable
			{
				private static readonly string _compilerCommandsFilename = "CompilerCommands.txt";
				
				public IEnumerator GetEnumerator() => File
				                                      .ReadAllLines(_compilerCommandsFilename)
				                                      .Select(cmd => new object[] {new ProcessCommandInterface(cmd)})
				                                      .Prepend(new object[] {new SelfCommandInterface()})
				                                      .GetEnumerator();
				
				/// <summary>
				/// Implements commands directly.
				/// </summary>
				private class SelfCommandInterface : ICommandInterface
				{
					public override string ToString() => "Laboratory";

					public CommandContext Context { get; } = new CommandContext();
					
					public float[] Execute(string functionName, params float[] functionArgs)
					{
						var sourceContext = new SourceContext();
						var compilationContext = new CompilationContext(logToConsole: false);

						FileInfo[] OpenPackage(string package) =>new DirectoryInfo(package).GetFiles("*.ele", SearchOption.AllDirectories);
						if (Context.IncludePrelude) sourceContext.AddSourceFiles(OpenPackage("StandardLibrary"));
						sourceContext.AddSourceFiles(Context.Packages.SelectMany(OpenPackage));
						
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

						Context.MessageHandler(messages, anyErrors);
						
						return result;
					}
				}

				/// <summary>
				/// Implements commands by calling external process defined using a command string.
				/// </summary>
				private class ProcessCommandInterface : ICommandInterface
				{
					public ProcessCommandInterface(string command)
					{
						_command = command;
					}

					public override string ToString() => $"Process: <{_command}>";

					private readonly string _command;

					public CommandContext Context { get; } = new CommandContext();

					public float[] Execute(string functionName, params float[] functionArgs)
					{
						var processArgs = new StringBuilder();
						processArgs.Append("execute");
						var packages = Context.Packages;
						if (Context.IncludePrelude || packages?.Length > 0)
						{
							processArgs.Append(" -p ");
							if (Context.IncludePrelude) processArgs.Append("StandardLibrary ");
							if (packages?.Length > 0) processArgs.AppendJoin(' ', packages);
						}

						processArgs.Append(" -f ");
						processArgs.Append(functionName);

						if (functionArgs?.Length > 0)
						{
							processArgs.Append(" -a ");
							processArgs.AppendJoin(' ', functionArgs);
						}
					
						var process = new Process
						{
							StartInfo = new ProcessStartInfo
							{
								FileName = _command,
								Arguments = processArgs.ToString(),
								RedirectStandardOutput = true,
								RedirectStandardError = true
							},
							EnableRaisingEvents = true
						};

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

						if (process.ExitCode != 0)
						{
							messages.PrintMessagesToTestContext();
							Assert.Inconclusive("Compiler process quit with unexpected exit code '{0}'.", process.ExitCode);
						}
						
						Context.MessageHandler(messages, anyErrors);

						return messages.Last().Split(' ', StringSplitOptions.RemoveEmptyEntries).Select(s => float.Parse(s, CultureInfo.InvariantCulture)).ToArray();
					}
				}
			}
		}
	}
}