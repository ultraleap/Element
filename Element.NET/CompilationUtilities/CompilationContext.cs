using System.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using Tomlyn;
using Tomlyn.Model;

namespace Element
{
	/// <summary>
	/// Contains the status of the compilation process including call stack and logging messages.
	/// </summary>
	public class CompilationContext
	{
		private CompilationContext(in CompilationInput compilationInput)
		{
			Input = compilationInput;
			LogCallback = compilationInput.LogCallback;
		}

		public static bool TryCreate(in CompilationInput compilationInput, out CompilationContext compilationContext) =>
			(compilationContext = new CompilationContext(compilationInput))
			.ParseFiles(compilationInput.Packages
				.Prepend(compilationInput.ExcludePrelude ? null : new DirectoryInfo("Prelude"))
				.SelectMany(directory => directory?.GetFiles("*.ele", SearchOption.AllDirectories) ?? Array.Empty<FileInfo>())
				.Concat(compilationInput.ExtraSourceFiles)
				.ToArray())
			.All(parseResult => parseResult.Success);

		public CompilationInput Input { get; }
		public GlobalScope GlobalScope { get; } = new GlobalScope();

		private static readonly TomlTable _messageToml = Toml.Parse(File.ReadAllText("Messages.toml")).ToModel();

		private Action<CompilerMessage> LogCallback { get; }

		private readonly Stack<TraceSite> _callStack = new Stack<TraceSite>();
		public void Push(TraceSite traceSite) => _callStack.Push(traceSite);
		public void Pop() => _callStack.Pop();

		public IFunction LogError(int messageCode, string context = default) => LogImpl(messageCode, context);
		public void Log(string message) => LogImpl(null, message);
		private IFunction LogImpl(int? messageCode, string context = default)
		{
			if (!messageCode.HasValue)
			{
				LogCallback?.Invoke(new CompilerMessage(null, null, null, context, DateTime.Now, _callStack.ToArray()));
				return CompilationError.Instance;
			}
			
			if(!(_messageToml[$"ELE{messageCode}"] is TomlTable messageTable))
			{
				throw new InternalCompilerException($"ELE{messageCode} could not be found");
			}


			if (!Enum.TryParse((string)messageTable["level"], out MessageLevel level))
			{
				throw new InternalCompilerException($"\"{level}\" is not a valid message level");
			}

			if (level >= Input.Verbosity)
			{
				LogCallback?.Invoke(new CompilerMessage(messageCode.Value, (string)messageTable["name"], level, context, DateTime.Now, _callStack.ToArray()));
			}
			
			return CompilationError.Instance;
		}
	}
}