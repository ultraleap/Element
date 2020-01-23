using System;
using System.IO;

namespace Laboratory
{
	/// <summary>
	/// Utilities and extension functions
	/// </summary>
	internal static class Laboratory
	{
		internal const float FloatEpsilon = 1.19209e-7f;

		internal static readonly Lazy<string> ElementRootDirectory = new Lazy<string>(() =>
		{
			var success = new DirectoryInfo(Directory.GetCurrentDirectory()).TryGetParent("Element", out var rootDir);
			if (!success) throw new DirectoryNotFoundException("Couldn't find Element root directory");
			return rootDir.FullName;
		});

		private static bool TryGetParent(this DirectoryInfo directory, string parentName, out DirectoryInfo parent)
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
	}
}