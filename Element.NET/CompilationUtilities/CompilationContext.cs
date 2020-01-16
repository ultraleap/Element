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
		public CompilationContext(string compilerflagsToml = "CompilerFlags.toml", bool logToConsole = true)
		{
			_compilerFlags = Toml.Parse(File.ReadAllText(compilerflagsToml)).ToModel();
			_callStack = new Stack<CallSite>();
			if (logToConsole) this.StartLoggingToConsole();
		}

		private readonly TomlTable _compilerFlags;
		private static readonly TomlTable _messageToml = Toml.Parse(File.ReadAllText("Messages.toml")).ToModel();
		private static readonly string[] _messageLevels = ((TomlArray)_messageToml["levels"]).Select(i => (string)i).ToArray();

		private TValue CompilerFlag<TValue>([CallerMemberName] string caller = default) => (TValue)((TomlTable)_compilerFlags[caller ?? throw new ArgumentNullException(nameof(caller))])["value"];

		public bool Debug => CompilerFlag<bool>();
		public string Verbosity => CompilerFlag<string>();

		public event LogEvent OnLog;
		public event LogEvent OnError;

		private readonly Stack<CallSite> _callStack;
		public void Push(CallSite callSite) => _callStack.Push(callSite);
		public void Pop() => _callStack.Pop();

		public IFunction LogError(int messageCode, string context = default) => LogImpl(messageCode, true, context);
		public void Log(string message) => LogImpl(null, false, message);
		private IFunction LogImpl(int? messageCode, bool appendStackTrace = false, string context = default)
		{
			var sb = new StringBuilder();
			string level;
			if (messageCode.HasValue && _messageToml[$"ELE{messageCode.Value}"] is TomlTable messageTable)
			{
				level = (string)messageTable["level"];
				sb.Append("ELE").Append(messageCode).Append(": ").Append(level).Append(" - ").Append((string)messageTable["name"]).AppendLine();
				sb.AppendLine((string)messageTable["summary"]);
			}
			else
			{
				throw new InternalCompilerException($"ELE{messageCode} could not be found");
			}
			
			var indexOfLevel = Array.IndexOf(_messageLevels, level);
			if (indexOfLevel < 0) throw new InternalCompilerException($"Couldn't find {level} in message levels array");

			var indexOfCurrentVerbosity = Array.IndexOf(_messageLevels, Verbosity);
			if (indexOfCurrentVerbosity < 0) throw new InternalCompilerException($"Couldn't find {Verbosity} in message levels array");

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
			}
			
			return Error.Instance;
		}
	}

	public delegate void LogEvent(string message);

	public class InternalCompilerException : Exception
	{
		public InternalCompilerException(string message)
			: base(message) { }
	}

	public static class LogToConsole
	{
		public static void StartLoggingToConsole(this CompilationContext context)
		{
			StopLoggingToConsole(context); // Prevent duplicate subscriptions
			context.OnLog += LogToStdOut;
			context.OnError += LogToStdErr;
		}

		public static void StopLoggingToConsole(this CompilationContext context)
		{
			context.OnLog -= LogToStdOut;
			context.OnError -= LogToStdErr;
		}

		private static void LogToStdErr(string message) => Console.Error.WriteLine(message);
		private static void LogToStdOut(string message) => Console.WriteLine(message);
	}
}