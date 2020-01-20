namespace Element
{
	using System;
	using System.Text;
	using System.Linq;
	using System.Runtime.CompilerServices;
	using System.Collections.Generic;
	using System.IO;
	using Tomlyn;
	using Tomlyn.Model;

	/// <summary>
	/// Contains the state of the compiler including compilation flags and the compilation stack.
	/// Also provides capabilities for logging messages to the user.
	/// </summary>
	public class CompilationContext
	{
		public CompilationContext(string compilerflagsToml = "CompilerFlags.toml")
		{
			_compilerFlags = Toml.Parse(File.ReadAllText(compilerflagsToml)).ToModel();
			_callStack = new Stack<CallSite>();
		}

		private readonly TomlTable _compilerFlags;
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

		private TValue CompilerFlag<TValue>([CallerMemberName] string caller = default) => (TValue)((TomlTable)_compilerFlags[caller ?? throw new ArgumentNullException(nameof(caller))])["value"];

		public bool Debug => CompilerFlag<bool>();
		public string Verbosity => CompilerFlag<string>();

		public bool LogToConsole { get; set; } = true;

		public readonly List<CompilerMessage> Messages = new List<CompilerMessage>();

		private readonly Stack<CallSite> _callStack;
		public void Push(CallSite callSite) => _callStack.Push(callSite);
		public void Pop() => _callStack.Pop();

		public IFunction LogError(int messageCode, string context = default) => LogImpl(messageCode, true, context);
		public void Log(string message) => LogImpl(null, false, message);
		private IFunction LogImpl(int? messageCode, bool appendStackTrace = false, string context = default)
		{
			TomlTable messageDetails = null;
			if (messageCode.HasValue) messageDetails = GetMessageCode(messageCode.Value);

			var indexOfLevel = Enum.IndexOf(_messageLevels, level);
			if (indexOfLevel < 0) throw new InternalCompilerException($"\"{level}\" is not a valid message level");

			var indexOfCurrentVerbosity = Array.IndexOf(_messageLevels, Verbosity);
			if (indexOfCurrentVerbosity < 0) throw new InternalCompilerException($"\"{Verbosity}\" is not a valid message level");

			if (indexOfLevel >= indexOfCurrentVerbosity)
			{
				sb.AppendLine(context);
				sb.AppendLine();
				if (appendStackTrace)
				{
					sb.AppendLine("Stack trace:");
					foreach (var frame in _callStack)
					{
						sb.Append("    ").AppendLine(frame.ToString());
					}
				}
				
				var message = sb.ToString();
				var isError = Array.IndexOf(_messageLevels, "Error") >= indexOfLevel;
				if (isError) OnError?.Invoke(message);
				else OnLog?.Invoke(message);

				if (LogToConsole)
				{
					if (isError) Console.Error.WriteLine(message);
					else Console.WriteLine(message);
				}
			}
			
			return CompileError.Instance;
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

	public struct CompilerMessage
	{
		public CompilerMessage(string context, MessageLevel level)
		{
			Level = level;
			TimesStamp = DateTime.Now;
		}

		private readonly StringBuilder _builder;

		public override string ToString()
		{
			var builder = new StringBuilder();
			builder.Append("ELE").Append(messageCode).Append(": ").Append(level).Append(" - ").Append((string)messageTable["name"]).AppendLine();
			return _builder.ToString();
		}

		public int MessageCode { get; }
		public Stack<CallSite> CallStack { get; }
		public MessageLevel Level { get; }
		public DateTime TimesStamp { get; }
	}

	public class InternalCompilerException : Exception
	{
		public InternalCompilerException(string message)
			: base(message) { }
	}
}