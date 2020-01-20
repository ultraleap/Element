namespace Alchemist
{
	using System.Collections.Generic;
	using System.IO;
	using System.Linq;
	using CommandLine;

	/// <summary>
	/// Base class for writing Alchemist commands.
	/// </summary>
	internal abstract class BaseCommand
	{
		[Option('p', "packages", Required = false, HelpText = "Element packages to load into the context.")]
		public IEnumerable<string> Packages { get; set; }

		private readonly string _currentWorkingDirectory = Directory.GetCurrentDirectory();

		private HostContext InitializeHostContext()
		{
			Alchemist.Log($"Alchemist starting in directory \"{_currentWorkingDirectory}\"");

			return new HostContext
			{
				Packages = Packages.Select(GetPackageDirectories).Prepend(new DirectoryInfo(_currentWorkingDirectory)).ToList(),
				IncludePrelude = true,
				MessageHandler = Alchemist.Log,
				ErrorHandler = Alchemist.LogError
			};
		}

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

		public int Invoke() => CommandImplementation(InitializeHostContext());

		protected abstract int CommandImplementation(HostContext hostContext);
	}
}