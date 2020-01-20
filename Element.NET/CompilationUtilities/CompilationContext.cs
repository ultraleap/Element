namespace Element
{
	using System;
	using System.Text;
	using System.Collections.Generic;
	using System.IO;
	using Tomlyn;
	using Tomlyn.Model;

	/// <summary>
	/// Contains the status of the compilation process including call stack and logging messages.
	/// </summary>
	public class CompilationContext
	{
		public CompilationContext(in CompilationInput compilationInput) => CompilationInput = compilationInput;

		public CompilationInput CompilationInput { get; }
		
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
		public IReadOnlyList<CompilerMessage> Messages => _messages.AsReadOnly();
		private List<CompilerMessage> _messages { get; } = new List<CompilerMessage>();

		private readonly Stack<CallSite> _callStack = new Stack<CallSite>();
		public void Push(CallSite callSite) => _callStack.Push(callSite);
		public void Pop() => _callStack.Pop();

		public IFunction LogError(int messageCode, string context = default) => LogImpl(messageCode, true, context);
		public void Log(string message) => LogImpl(null, false, message);
		private IFunction LogImpl(int? messageCode, bool appendStackTrace = false, string context = default)
		{
			if (!messageCode.HasValue)
			{
				_messages.Add(new CompilerMessage(null, null, null, context, _callStack, appendStackTrace));
				return CompilationError.Instance;
			}
			
			var messageDetails = GetMessageCode(messageCode.Value);
			if (!Enum.TryParse((string)messageDetails["level"], out MessageLevel level))
			{
				throw new InternalCompilerException($"\"{level}\" is not a valid message level");
			}

			if (level >= CompilationInput.Verbosity)
			{
				var message = new CompilerMessage(messageCode.Value, (string)messageDetails["name"], level, context, _callStack, appendStackTrace);
				_messages.Add(message);
				if (CompilationInput.LogToConsole)
				{
					if (MessageLevel.Error >= level) Console.Error.WriteLine(message);
					else Console.WriteLine(message);
				}
			}
			
			return CompilationError.Instance;
		}
	}

	internal static class CompilationExtensions
	{
		public static CompilationResult<TResult> ToResult<TResult>(this CompilationContext context, TResult result) =>
			new CompilationResult<TResult>(result, context);
		
		public static Stack<T> Clone<T>(this Stack<T> original)
		{
			var arr = new T[original.Count];
			original.CopyTo(arr, 0);
			Array.Reverse(arr);
			return new Stack<T>(arr);
		}
	}

	public enum MessageLevel
	{
		Verbose,
		Information,
		Warning,
		Error,
		Fatal
	}

	public class CompilerMessage
	{
		public CompilerMessage(int? messageCode, string? name, MessageLevel? messageLevel, string context, Stack<CallSite> callStack, bool appendStackTrace)
		{
			Name = name;
			Level = messageLevel;
			CallStack = callStack.Clone();
			_message = new Lazy<string>(() =>
			{
				var builder = new StringBuilder();
				if (messageCode.HasValue)
				{
					builder.Append("ELE").Append(messageCode.Value).Append(": ").Append(Level).Append(" - ")
					       .Append(Name).AppendLine();
				}

				builder.AppendLine(context);
				builder.AppendLine();
				if (appendStackTrace)
				{
					builder.AppendLine("Stack trace:");
					foreach (var frame in CallStack)
					{
						builder.Append("    ").AppendLine(frame.ToString());
					}
				}
				return builder.ToString();
			});
		}
		private readonly Lazy<string> _message;

		public string? Name { get; }
		public MessageLevel? Level { get; }
		public Stack<CallSite> CallStack { get; }

		public override string ToString() => _message.Value;
	}

	public class InternalCompilerException : Exception
	{
		public InternalCompilerException(string message)
			: base(message) { }
	}
}