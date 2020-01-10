using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using NUnit.Framework;

namespace Laboratory
{
	/// <summary>
	/// Utilities and extension functions
	/// </summary>
	internal static class Laboratory
	{
		internal static void Check(this List<string> messages, string messageCode)
		{
			if (string.IsNullOrEmpty(messageCode)) throw new ArgumentNullException(nameof(messageCode));
			Assert.True(messages.Any(m => m.StartsWith(messageCode)));
		}

		internal static void PrintMessagesToTestContext(this List<string> messages, string what)
		{
			TestContext.WriteLine($"*** BEGIN: {what} ***");
			foreach (var message in messages.Where(message => !string.IsNullOrEmpty(message)))
			{
				TestContext.WriteLine(message);
			}

			TestContext.WriteLine($"*** END: {what} ***");
		}

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