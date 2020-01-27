using System;
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
		[Option("no-prelude", HelpText = "Prelude functionality is included. Warning: Without Prelude only unverified compiler intrinsics will be available.")]
		public bool ExcludePrelude { get; set; }

		[Option("packages", Required = false, HelpText = "Element packages to load into the context.")]
		public IEnumerable<string> Packages { get; set; }

		[Option("logjson", Required = false, HelpText = "Serializes log messages structured as Json instead of plain string.")]
		public bool LogMessagesAsJson { get; set; }

		private DirectoryInfo GetPackageDirectories(string package)
		{
			var directoryInfo = Directory.Exists(package) ? new DirectoryInfo(package) : null;
			if (directoryInfo != null) return directoryInfo;
			Log($"Package directory \"{package}\" doesn't exist.", MessageLevel.Error);
			return null;
		}

		public int Invoke(string args)
		{
			Log(args);
			var (exitCode, result) = CommandImplementation(new CompilationInput(Log, ExcludePrelude, Packages.Select(GetPackageDirectories).ToList()));
			Log(result);
			return exitCode;
		}

		private void Log(string message, MessageLevel? level = default) => Log(new CompilerMessage(message, DateTime.Now, level));

		private void Log(CompilerMessage message)
		{
			var msg = LogMessagesAsJson ? JsonConvert.SerializeObject(message) : message.ToString();
			if (message.MessageLevel >= MessageLevel.Error) Console.Error.Write(msg);
			else Console.Write(msg);
		}

		protected abstract (int ExitCode, string Result) CommandImplementation(in CompilationInput input);
	}
}