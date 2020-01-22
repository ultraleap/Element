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
				.SelectMany(directory => directory?.GetFiles("*.ele", SearchOption.AllDirectories))
				.Concat(compilationInput.ExtraSourceFiles)
				.ToArray())
			.All(parseResult => parseResult.Success);

		public CompilationInput Input { get; }
		public GlobalScope GlobalScope { get; } = new GlobalScope();

		private static readonly TomlTable _messageToml = Toml.Parse(File.ReadAllText("Messages.toml")).ToModel();
		private static readonly Dictionary<int, TomlTable> _messageDetails = new Dictionary<int, TomlTable>();

		private static TomlTable GetMessageCode(int messageCode)
		{
			if (_messageDetails.TryGetValue(messageCode, out var result))
			{
				return result;
			}

			if(_messageToml[$"ELE{messageCode}"] is TomlTable messageTable)
			{
				return _messageDetails[messageCode] = messageTable;
			}

			throw new InternalCompilerException($"ELE{messageCode} could not be found");
		}

		private Action<CompilerMessage> LogCallback { get; }

		private readonly Stack<CallSite> _callStack = new Stack<CallSite>();
		public void Push(CallSite callSite) => _callStack.Push(callSite);
		public void Pop() => _callStack.Pop();

		public IFunction LogError(int messageCode, string context = default) => LogImpl(messageCode, context);
		public void Log(string message) => LogImpl(null, message);
		private IFunction LogImpl(int? messageCode, string context = default)
		{
			if (!messageCode.HasValue)
			{
				LogCallback?.Invoke(new CompilerMessage(null, null, null, context, _callStack.ToArray()));
				return CompilationError.Instance;
			}
			
			var messageDetails = GetMessageCode(messageCode.Value);
			if (!Enum.TryParse((string)messageDetails["level"], out MessageLevel level))
			{
				throw new InternalCompilerException($"\"{level}\" is not a valid message level");
			}

			if (level >= Input.Verbosity)
			{
				LogCallback?.Invoke(new CompilerMessage(messageCode.Value, (string)messageDetails["name"], level, context, _callStack.ToArray()));
			}
			
			return CompilationError.Instance;
		}
	}
}