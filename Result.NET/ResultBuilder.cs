using System.Collections.Generic;
using System.Linq;

namespace ResultNET
{
    public class ResultBuilder
    {
        private readonly List<ResultMessage> _messages;

        public ResultBuilder(ITraceContext context)
        {
            Context = context;
            _messages = new List<ResultMessage>();
        }

        public ITraceContext Context { get; }
        public IReadOnlyList<ResultMessage> Messages => _messages;

        public void Append(MessageInfo messageInfo, string? context) => Append(Context.Trace(messageInfo, context));
        public void Append(ResultMessage? message)
        {
            if (message != null) _messages.Add(message);
        }

        public void AppendVerbose(string message) => Append(Context.Trace(MessageLevel.Verbose, message));
        public void AppendInfo(string message) => Append(Context.Trace(MessageLevel.Information, message));
        public void Append(Result result) => _messages.AddRange(result.Messages);
        public void Append<T>(in Result<T> result) => _messages.AddRange(result.Messages);

        public Result ToResult() => new Result(_messages);
    }
    
    public class ResultBuilder<T>
    {
        private readonly List<ResultMessage> _messages;

        public ResultBuilder(ITraceContext context, T initial)
        {
            Context = context;
            _messages = new List<ResultMessage>();
            Result = initial;
        }
        
        public ITraceContext Context { get; }
        public T Result { get; set; }
        public IReadOnlyList<ResultMessage> Messages => _messages;

        public void Append(MessageInfo messageInfo, string? context) => Append(Context.Trace(messageInfo, context));
        public void Append(ResultMessage? message)
        {
            if (message != null) _messages.Add(message);
        }

        public void AppendVerbose(string message) => Append(Context.Trace(MessageLevel.Verbose, message));
        public void AppendInfo(string message) => Append(Context.Trace(MessageLevel.Information, message));
        public void Append(Result result) => _messages.AddRange(result.Messages);
        public void Append<TResult>(in Result<TResult> result) => _messages.AddRange(result.Messages);
        public void Append(IEnumerable<ResultMessage> messages) => _messages.AddRange(messages);
        public void Append(IReadOnlyCollection<ResultMessage> messages) => _messages.AddRange(messages);

        public Result<T> ToResult() => Result == null && !Messages.Any(m => m.Info.Level >= MessageLevel.Error)
            ? new Result<T>(Context.Trace(MessageLevel.Error, "Cannot cast null value to Result type") is {} error
                ? (IReadOnlyCollection<ResultMessage>)_messages.Append(error).ToArray()
                : _messages)
            : new Result<T>(Result, _messages);
    }
}