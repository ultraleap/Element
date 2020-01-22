using Element;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using CommandLine;
using Newtonsoft.Json;

namespace Alchemist
{
	/// <summary>
	/// Base class for writing Alchemist commands.
	/// </summary>
	internal abstract class BaseCommand
	{
		[Option("prelude", Default = true, HelpText = "Prelude functionality is included. Warning: Without Prelude only unverified compiler intrinsics will be available.")]
		public bool IncludePrelude { get; set; }

		[Option("packages", Required = false, HelpText = "Element packages to load into the context.")]
		public IEnumerable<string> Packages { get; set; }

		[Option("logjson", Required = false, HelpText = "Serializes log messages structured as Json instead of plain string.")]
		public bool LogMessagesAsJson { get; set; }

		private static DirectoryInfo GetPackageDirectories(string package)
		{
			var directoryInfo = Directory.Exists(package) ? new DirectoryInfo(package) : null;
			if (directoryInfo != null) return directoryInfo;
			Alchemist.LogError($"Package directory \"{package}\" doesn't exist.");
			return null;
		}

		public int Invoke()
		{
			var (exitCode, result) = CommandImplementation(new CompilationInput(LogCallback, !IncludePrelude,
				Packages.Select(GetPackageDirectories).ToList()));
			Alchemist.Log(result);
			return exitCode;
		}

		private void LogCallback(CompilerMessage message)
		{
			var msg = LogMessagesAsJson ? message.ToString() : JsonConvert.SerializeObject(message);
			if (message.Level >= MessageLevel.Error) Alchemist.LogError(msg);
			else Alchemist.Log(msg);
		}

		protected abstract (int ExitCode, string Result) CommandImplementation(in CompilationInput input);
	}
}