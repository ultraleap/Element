using System;
using System.Collections.Generic;
using System.Linq;

namespace Element
{
    public class ResultBuilder
    {
        private readonly ITrace _trace;
        private readonly List<CompilerMessage> _messages;

        public ResultBuilder(ITrace trace)
        {
            _trace = trace;
            _messages = new List<CompilerMessage>();
        }

        public void Append(CompilerMessage message) => _messages.Add(message);
        public void Append(MessageCode messageCode, string context) => _messages.Add(_trace.Trace(messageCode, context));
        public void Append(Result result) => _messages.AddRange(result.Messages);

        public Result ToResult() => new Result(_messages);
    }

    public class ResultBuilder<T> where T : notnull
    {
        private readonly ITrace _trace;
        private readonly List<CompilerMessage> _messages;

        public ResultBuilder(ITrace trace, T initial)
        {
            _trace = trace;
            _messages = new List<CompilerMessage>();
            Result = initial;
        }
        public T Result { get; set; }
        
        public void Append(CompilerMessage message) => _messages.Add(message);
        public void Append(MessageCode messageCode, string context) => _messages.Add(_trace.Trace(messageCode, context));
        public void Append(Result result) => _messages.AddRange(result.Messages);
        public void Append(Result<T> result) => _messages.AddRange(result.Messages);
        public void Append(IEnumerable<CompilerMessage> messages) => _messages.AddRange(messages);

        public Result<T> ToResult() => Result == null ? new Result<T>(_messages) : new Result<T>(Result, _messages);
    }
    
    public readonly struct Result
    {
        public static Result Success { get; } = new Result();
        
        public Result(CompilerMessage? message) : this(message == null ? Array.Empty<CompilerMessage>() : new []{message}) {}
        public Result(IReadOnlyCollection<CompilerMessage> messages)
        {
            IsError = messages.Any(m => m.MessageLevel >= MessageLevel.Error);
            Messages = messages;
        }
        
        public static implicit operator Result(CompilerMessage? message) => new Result(message);

        public bool IsSuccess => !IsError;
        public bool IsError { get; } // Defaults to false, default Result constructor will be success
        public IReadOnlyCollection<CompilerMessage> Messages { get; }
            
        public TResult Match<TResult>(Func<IReadOnlyCollection<CompilerMessage>, TResult> onResult, Func<IReadOnlyCollection<CompilerMessage>, TResult> onError) => IsSuccess ? onResult(Messages) : onError(Messages);
        
        public Result And(Func<Result> action) => IsSuccess ? new Result(Messages.Combine(action().Messages)) : this;
        public Result And(Func<IReadOnlyCollection<CompilerMessage>, Result> action) => IsSuccess ? action(Messages) : this;
        
        public Result<TResult> Map<TResult>(Func<TResult> mapFunc) where TResult : notnull => IsSuccess ? Merge(new Result<TResult>(mapFunc())) : new Result<TResult>(Messages);
        public Result<TResult> Map<TResult>(Func<IReadOnlyCollection<CompilerMessage>, TResult> mapFunc) where TResult : notnull => IsSuccess ? new Result<TResult>(mapFunc(Messages)) : new Result<TResult>(Messages);

        public Result<TResult> Bind<TResult>(Func<Result<TResult>> bindFunc) where TResult : notnull => IsSuccess ? Merge(bindFunc()) : new Result<TResult>(Messages);
        public Result<TResult> Bind<TResult>(Func<IReadOnlyCollection<CompilerMessage>, Result<TResult>> bindFunc) where TResult : notnull => IsSuccess ? bindFunc(Messages) : new Result<TResult>(Messages);
        
        public Result<TResult> Merge<TResult>(Result<TResult> newResult) where TResult : notnull => new Result<TResult>(newResult, Messages.Combine(newResult.Messages));
    }

    public static class ResultExtensions
    {
        public static Result Fold(this IEnumerable<Result> results)
        {
            results = results.ToArray();
            var messages = new CompilerMessage[results.Sum(r => r.Messages.Count)];
            var i = 0;
            foreach (var result in results)
            {
                foreach (var msg in result.Messages)
                {
                    messages[i] = msg;
                    i++;
                }
            }
            return new Result(messages);
        }

        public static IReadOnlyCollection<CompilerMessage> Combine(this IReadOnlyCollection<CompilerMessage> a, IReadOnlyCollection<CompilerMessage> b) =>
            (a.Count, b.Count) switch
            {
                (0, 0) => Array.Empty<CompilerMessage>(),
                (0, _) => b,
                (_, 0) => a,
                _ => a.Concat(b).ToArray()
            };

        public static Result Fold<TIn>(this IEnumerable<Result<TIn>> enumerable) where TIn : notnull => enumerable.Select(r => (Result)r).Fold();

        public static Result<TResult> BindEnumerable<TIn, TResult>(this IEnumerable<Result<TIn>> enumerable, Func<IEnumerable<TIn>, Result<TResult>> bindFunc)
            where TIn : notnull
            where TResult : notnull
        {
            var inputs = enumerable.ToArray();
            return inputs.Any(i => i.IsError)
                       ? new Result<TResult>(inputs.Fold().Messages)
                       : inputs.Fold().Merge(bindFunc(inputs.Select(i => i.ResultOr(default)))); // Or value will never be selected, we already checked for any errors
        }
        
        public static Result<TResult> MapEnumerable<TIn, TResult>(this IEnumerable<Result<TIn>> enumerable, Func<IEnumerable<TIn>, TResult> mapFunc)
            where TIn : notnull
            where TResult : notnull
        {
            var inputs = enumerable.ToArray();
            return inputs.Any(i => i.IsError)
                       ? new Result<TResult>(inputs.Fold().Messages)
                       : inputs.Fold().Merge(new Result<TResult>(mapFunc(inputs.Select(i => i.ResultOr(default))))); // Or value will never be selected, we already checked for any errors
        }
    }
    
    public readonly struct Result<T> : IEquatable<Result<T>> where T : notnull
    {
        public Result(T value)
        {
            IsSuccess = true;
            _value = value;
            Messages = Array.Empty<CompilerMessage>();
        }
        
        public Result(T value, IReadOnlyCollection<CompilerMessage> messages)
        {
            IsSuccess = !messages.Any(m => m.MessageLevel >= MessageLevel.Error);
            _value = value;
            Messages = messages;
        }

        public Result(T value, CompilerMessage? message) :this(value, message == null ? Array.Empty<CompilerMessage>() : new []{message}) {}
        public Result(Result<T> value, CompilerMessage? message) :this(value._value, message == null ? value.Messages : value.Messages.Append(message).ToArray()) { }
        public Result(Result<T> value, IReadOnlyCollection<CompilerMessage> messages) :this(value._value, messages) { }

        public Result(CompilerMessage? message) : this(message == null ? Array.Empty<CompilerMessage>() : new []{message}) { }

        public Result(IReadOnlyCollection<CompilerMessage> messages)
        {
            IsSuccess = false; // No value was passed, a result with no messages is an error whether there are any error messages or not
            
#pragma warning disable 8601
            _value = default; // Might assign null if T is reference type but there's no way to retrieve this accidentally so it's ok!
#pragma warning restore 8601
            
            Messages = messages;
        }
        
        public static explicit operator Result(Result<T> result) => new Result(result.Messages);
        public static implicit operator Result<T>(T v) => new Result<T>(v);
        public static implicit operator Result<T>(CompilerMessage? message) => new Result<T>(message);
        
        public bool IsSuccess { get; } // Defaults to false, default constructor is creates an error Result<T>
        public bool IsError => !IsSuccess;
        public IReadOnlyCollection<CompilerMessage> Messages { get; }
        private readonly T _value;
        
        public TResult Match<TResult>(Func<T, IReadOnlyCollection<CompilerMessage>, TResult> onResult, Func<IReadOnlyCollection<CompilerMessage>, TResult> onError) => IsSuccess ? onResult(_value, Messages) : onError(Messages);
        
        public Result Do(Func<T, Result> action) => IsSuccess ? new Result(Messages.Combine(action(_value).Messages)) : new Result(Messages);
        public Result Do(Func<T, IReadOnlyCollection<CompilerMessage>, Result> action) => IsSuccess ? action(_value, Messages) : new Result(Messages);
        
        public Result Do(Action<T> action)
        {
            if (IsSuccess) action(_value);
            return (Result)this;
        }

        public Result Do(Action<T, IReadOnlyCollection<CompilerMessage>> action)
        {
            if (IsSuccess) action(_value, Messages);
            return (Result) this;
        }

        public Result<TResult> Map<TResult>(Func<T, TResult> mapFunc) where TResult : notnull => IsSuccess ? Merge(new Result<TResult>(mapFunc(_value))) : new Result<TResult>(Messages);
        public Result<TResult> Map<TResult>(Func<T, IReadOnlyCollection<CompilerMessage>, TResult> mapFunc) where TResult : notnull => IsSuccess ? new Result<TResult>(mapFunc(_value, Messages)) : new Result<TResult>(Messages);
        
        public Result<TResult> Bind<TResult>(Func<T, Result<TResult>> bindFunc) where TResult : notnull  => IsSuccess ? Merge(bindFunc(_value)) : new Result<TResult>(Messages);
        public Result<TResult> Bind<TResult>(Func<T, IReadOnlyCollection<CompilerMessage>, Result<TResult>> bindFunc) where TResult : notnull => IsSuccess ? bindFunc(_value, Messages) : new Result<TResult>(Messages);

        public Result<T> Else(Func<T> elseFunc) => IsSuccess ? this : new Result<T>(elseFunc());
        public Result<T> Else(Func<Result<T>> elseFunc) => IsSuccess ? this : elseFunc();
        public Result<T> ElseIf(bool condition, Func<T> elseFunc) => condition && !IsSuccess ? elseFunc() : this;
        public Result<T> ElseIf(bool condition, Func<Result<T>> elseFunc) => condition && !IsSuccess ? elseFunc() : this;
        
        public T ResultOr(T alternative) => IsSuccess ? _value : alternative;

        public Result<TResult> Merge<TResult>(Result<TResult> newResult) where TResult : notnull => new Result<TResult>(newResult._value, Messages.Combine(newResult.Messages));
        
        public bool Equals(Result<T> other) => IsSuccess && EqualityComparer<T>.Default.Equals(_value, other._value);

        public override bool Equals(object obj) =>
            !ReferenceEquals(null, obj)
            && obj.GetType() == GetType()
            && Equals((Result<T>) obj);

        public override int GetHashCode() => _value == null ? 0 : EqualityComparer<T>.Default.GetHashCode(_value);
    }
}