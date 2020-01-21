using Element;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using CommandLine;

namespace Alchemist
{
	/// <summary>
	/// Base class for writing Alchemist commands.
	/// </summary>
	internal abstract class BaseCommand
	{
		[Option('p', "packages", Required = false, HelpText = "Element packages to load into the context.")]
		public IEnumerable<string> Packages { get; set; }

		private static readonly string _currentWorkingDirectory = Directory.GetCurrentDirectory();

		private static DirectoryInfo GetPackageDirectories(string package)
		{
			var directoryInfo = Directory.Exists(package) ? new DirectoryInfo(package) : null;
			if (directoryInfo == null)
			{
				Alchemist.LogError($"Package directory \"{package}\" doesn't exist.");
				return null;
			}

			return directoryInfo;
		}

		public int Invoke() => CommandImplementation(new CompilationInput(true, true,
			Packages.Select(GetPackageDirectories).Prepend(new DirectoryInfo(_currentWorkingDirectory)).ToList(),
			Alchemist.Log, Alchemist.LogError));

		protected abstract int CommandImplementation(CompilationInput input);
	}
}