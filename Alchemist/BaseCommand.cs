using System;
using Element;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using CommandLine;
using Newtonsoft.Json;
using SemVer;

namespace Alchemist
{
	/// <summary>
	/// Base class for writing Alchemist commands.
	/// </summary>
	internal abstract class BaseCommand
	{
		[Option("no-prelude", Default = false, HelpText = "Prelude functionality is included. Warning: Without Prelude only unverified compiler intrinsics will be available.")]
		public bool ExcludePrelude { get; set; }
		
		[Option("prelude-version", Required = false, HelpText = "Version (as SemVer 2.0 version or npm style version range) of the Prelude to use. Uses the latest available version if none is specified. Ignored is ExcludePrelude is true.")]
		public string PreludeVersion { get; set; }

		[Option("packages", Required = false, HelpText = "Element packages, specified as name-version (where version is a SemVer 2.0 version or npm style version range), to load into the context.")]
		public IReadOnlyCollection<string> Packages { get; set; }

		[Option("source-files", Required = false, HelpText = "Extra individual source files, specified as paths, to load into the context.")]
		public IReadOnlyCollection<string> ExtraSourceFiles { get; set; }
		
		[Option("debug", Required = false, Default = false, HelpText = "Preserves debug information while compiling.")]
		public bool Debug { get; set; }
		
		[Option("verbosity", Required = false, Default = MessageLevel.Information, HelpText = "Verbosity of compiler messages.")]
		public MessageLevel Verbosity { get; set; }

		[Option("logjson", Required = false, Default = false, HelpText = "Serializes log messages structured as Json instead of plain string.")]
		public bool LogMessagesAsJson { get; set; }
		
		protected abstract bool _skipValidation { get; }
		
		protected abstract bool _noParseTrace { get; }

		public int Invoke(string args)
		{
			Log(args);

			var options = new CompilerOptions(!Debug, _skipValidation, _noParseTrace, Verbosity);
			var builder = new ResultBuilder(new Context(null, options));

			var currentDirectory = new DirectoryInfo(Directory.GetCurrentDirectory());
			var packageRegistry = new DirectoryPackageRegistry(currentDirectory);
			
			// Validate packages
			var packages = new List<PackageSpecifier>(Packages.Count);
			foreach (var pkg in Packages)
			{
				try
				{
					var nameAndVersion = pkg.Split('-', 2);
					packages.Add(new PackageSpecifier(nameAndVersion[0], new SemVer.Range(nameAndVersion[1])));
				}
				catch (Exception e)
				{
					builder.Append(MessageLevel.Error, e.ToString());
				}
			}

			// Validate source files
			var sourceFiles = new List<FileInfo>(ExtraSourceFiles.Count);
			foreach (var file in ExtraSourceFiles)
			{
				try
				{
					sourceFiles.Add(new FileInfo(file));
				}
				catch (Exception e)
				{
					builder.Append(MessageLevel.Error, e.ToString());
				}
			}

			var result = CommandImplementation(
				new CompilerInput(packageRegistry,
				                  ExcludePrelude
					                  ? null
					                  : new SemVer.Range(string.IsNullOrEmpty(PreludeVersion) ? "*" : PreludeVersion),
				                  packages,
				                  sourceFiles,
				                  options));
			foreach (var msg in result.Messages)
			{
				Log(msg);
			}

			if (result.IsSuccess) Log(result.ResultOr(string.Empty));
			return result.IsSuccess ? 0 : 1;
		}

		private void Log(string message, MessageLevel? level = default) => Log(new CompilerMessage(message, level));

		private void Log(CompilerMessage message)
		{
			var msg = LogMessagesAsJson ? JsonConvert.SerializeObject(message) : message.ToString();
			if (message.MessageLevel >= MessageLevel.Error) Console.Error.WriteLine(msg);
			else Console.WriteLine(msg);
		}

		protected abstract Result<string> CommandImplementation(CompilerInput input);
	}
}