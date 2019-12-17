namespace Alchemist
{
	using System;
	using Element;
	using System.Collections.Generic;
	using System.IO;
	using System.Linq;
	using CommandLine;
	using Element.CLR;

	/// <summary>
	/// Base class for writing Alchemist commands.
	/// </summary>
	internal abstract class BaseCommand
	{
		[Option('p', "packages", Required = false, HelpText = "Element packages to load into the context.")]
		public IEnumerable<string> Packages { get; set; }

		protected readonly SourceContext _sourceContext = new SourceContext();
		protected abstract CompilationContext _compilationContext { get; }
		private readonly string _currentWorkingDirectory = Directory.GetCurrentDirectory();

		private void InitializeContext()
		{
			Alchemist.Log($"Alchemist starting in directory \"{_currentWorkingDirectory}\"");

			var workingDirectoryFiles = new DirectoryInfo(_currentWorkingDirectory).GetFiles("*.ele");
			var packageFiles = Packages.SelectMany(GetPackageFiles);

			_sourceContext.AddSourceFiles(workingDirectoryFiles.Concat(packageFiles));
			_sourceContext.Recompile(_compilationContext);
		}

		// TODO(Craig): Make this more intelligent
		private FileInfo[] GetPackageFiles(string package)
		{
			var directoryInfo = Directory.Exists(package) ? new DirectoryInfo(package) : null;
			if (directoryInfo == null)
			{
				Alchemist.LogError($"Package directory \"{package}\" doesn't exist.");
				return Array.Empty<FileInfo>();
			}

			return directoryInfo.GetFiles("*.ele", SearchOption.AllDirectories);
		}

		public int Execute()
		{
			InitializeContext();
			return ExecuteImplementation();
		}

		protected abstract int ExecuteImplementation();
	}
}