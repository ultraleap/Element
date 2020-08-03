using System;
using System.Collections.Generic;
using System.Linq;

namespace Element
{
    public class ResultBuilder
    {
        private readonly List<CompilerMessage> _messages;

        public ResultBuilder(Context context)
        {
            Context = context;
            _messages = new List<CompilerMessage>();
        }

        public Context Context { get; }
        public IReadOnlyList<CompilerMessage> Messages => _messages;
        
        public void Append(CompilerMessage message) => _messages.Add(message);
        public void Append(MessageCode messageCode, string? context)
        {
            if (Context.Trace(messageCode, context) is {} msg) _messages.Add(msg);
        }
        public void Append(MessageLevel messageLevel, string message) => _messages.Add(Context.Trace(messageLevel, message));
        public void Append(Result result) => _messages.AddRange(result.Messages);
        public void Append<T>(in Result<T> result) => _messages.AddRange(result.Messages);

        public Result ToResult() => new Result(_messages);
    }

    public class ResultBuilder<T>
    {
        private readonly List<CompilerMessage> _messages;

        public ResultBuilder(Context context, T initial)
        {
            Context = context;
            _messages = new List<CompilerMessage>();
            Result = initial;
        }
        
        public Context Context { get; }
        public T Result { get; set; }
        public IReadOnlyList<CompilerMessage> Messages => _messages;
        
        public void Append(CompilerMessage message) => _messages.Add(message);
        public void Append(MessageCode messageCode, string? context)
        {
            if (Context.Trace(messageCode, context) is {} msg) _messages.Add(msg);
        }
        public void Append(MessageLevel messageLevel, string message) => _messages.Add(Context.Trace(messageLevel, message));
        public void Append(Result result) => _messages.AddRange(result.Messages);
        public void Append<TResult>(in Result<TResult> result) => _messages.AddRange(result.Messages);
        public void Append(IEnumerable<CompilerMessage> messages) => _messages.AddRange(messages);
        public void Append(IReadOnlyCollection<CompilerMessage> messages) => _messages.AddRange(messages);

        public Result<T> ToResult() => Result == null
                                           ? new Result<T>(Context.Trace(MessageCode.InvalidCast, "Cannot cast null value to Result type") is {} error
                                                               ? (IReadOnlyCollection<CompilerMessage>)_messages.Append(error).ToArray()
                                                               : _messages)
                                           : new Result<T>(Result, _messages);
    }
    
    public readonly struct Result
    {
        public static Result Success { get; } = new Result(Array.Empty<CompilerMessage>());
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
            
        public void Switch(Action<IReadOnlyCollection<CompilerMessage>> onResult, Action<IReadOnlyCollection<CompilerMessage>> onError)
        {
            if (IsSuccess) onResult(Messages);
            else onError(Messages);
        }
        public TResult Match<TResult>(Func<IReadOnlyCollection<CompilerMessage>, TResult> onResult, Func<IReadOnlyCollection<CompilerMessage>, TResult> onError) => IsSuccess ? onResult(Messages) : onError(Messages);
        public Result And(Func<Result> action) => IsSuccess ? new Result(Messages.Combine(action().Messages)) : this;
        public Result<TResult> Map<TResult>(Func<TResult> mapFunc) => IsSuccess ? Merge(new Result<TResult>(mapFunc())) : new Result<TResult>(Messages);
        public Result<TResult> Bind<TResult>(Func<Result<TResult>> bindFunc) => IsSuccess ? Merge(bindFunc()) : new Result<TResult>(Messages);
        public Result<TResult> Merge<TResult>(in Result<TResult> newResult) => new Result<TResult>(newResult, Messages.Combine(newResult.Messages));
    }

    public static class ResultExtensions
    {
        public static Result Fold(this IEnumerable<Result> results)
        {
            results = results as Result[] ?? results.ToArray();
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
        
        public static IReadOnlyCollection<CompilerMessage> Combine(this IReadOnlyCollection<CompilerMessage> a, params IReadOnlyCollection<CompilerMessage>[] others) =>
            a.Concat(others.SelectMany(m => m)).ToList();

        public static Result Fold<TIn>(this IEnumerable<Result<TIn>> enumerable) => enumerable.Select(r => (Result)r).Fold();

        public static Result<TResult> BindEnumerable<TIn, TResult>(this IEnumerable<Result<TIn>> enumerable, Func<IEnumerable<TIn>, Result<TResult>> bindFunc)
        {
            var inputs = enumerable as Result<TIn>[] ?? enumerable.ToArray();
            return inputs.Any(i => i.IsError)
                       ? new Result<TResult>(inputs.Fold().Messages)
                       : inputs.Fold().Merge(bindFunc(inputs.Select(i => i.ResultOr(default!)))); // Or value will never be selected, we already checked for any errors
        }
        
        public static Result<TResult> MapEnumerable<TIn, TResult>(this IEnumerable<Result<TIn>> enumerable, Func<IEnumerable<TIn>, TResult> mapFunc)
        {
            var inputs = enumerable as Result<TIn>[] ?? enumerable.ToArray();
            return inputs.Any(i => i.IsError)
                       ? new Result<TResult>(inputs.Fold().Messages)
                       : inputs.Fold().Merge(new Result<TResult>(mapFunc(inputs.Select(i => i.ResultOr(default!))))); // Or value will never be selected, we already checked for any errors
        }
        
        public static Result<TResult> FoldArray<TIn, TResult>(in this Result<TIn[]> enumerable, TResult initial, Func<TResult, TIn, Result<TResult>> foldFunc)
        {
            if (!enumerable.IsSuccess) return new Result<TResult>(enumerable.Messages);
            var inputs = enumerable.ResultOr(default!);
            List<CompilerMessage> messages = new List<CompilerMessage>(enumerable.Messages);
            var previous = foldFunc(initial, inputs[0]);
            messages.AddRange(previous.Messages);
            foreach (var item in inputs.Skip(1))
            {
                if (!previous.IsSuccess) return new Result<TResult>(messages);
                previous = foldFunc(previous.ResultOr(default!), item);
                messages.AddRange(previous.Messages);
            }

            return previous;
        }
        
        public static Result<IEnumerable<TResult>> ToResultEnumerable<TResult>(this IEnumerable<Result<TResult>> enumerable) =>
            MapEnumerable(enumerable, e => e);
        public static Result<TResult[]> ToResultArray<TResult>(this IEnumerable<Result<TResult>> enumerable) =>
            MapEnumerable(enumerable, e => e as TResult[] ?? e.ToArray());
        public static Result<IReadOnlyList<TResult>> ToResultReadOnlyList<TResult>(this IEnumerable<Result<TResult>> enumerable) =>
            MapEnumerable(enumerable, e => (IReadOnlyList<TResult>)(e as TResult[] ?? e.ToArray()));
    }
    
    public readonly struct Result<T> : IEquatable<Result<T>>
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
            _value = default!; // Might assign null if T is reference type but there's no way to retrieve it accidentally so it's ok!
            Messages = messages;
        }
        
        public static explicit operator Result(Result<T> result) => new Result(result.Messages);
        public static implicit operator Result<T>(T v) => new Result<T>(v);
        public static implicit operator Result<T>(CompilerMessage? message) => new Result<T>(message);

        public bool IsSuccess { get; } // Defaults to false, default constructor is creates an error Result<T>
        public bool IsError => !IsSuccess;
        public IReadOnlyCollection<CompilerMessage> Messages { get; }
        private readonly T _value;
        
        public void Switch(Action<T, IReadOnlyCollection<CompilerMessage>> onResult, Action<IReadOnlyCollection<CompilerMessage>> onError)
        {
            if (IsSuccess) onResult(_value, Messages);
            else onError(Messages);
        }

        public TResult Match<TResult>(Func<T, IReadOnlyCollection<CompilerMessage>, TResult> onResult, Func<IReadOnlyCollection<CompilerMessage>, TResult> onError) => IsSuccess ? onResult(_value, Messages) : onError(Messages);
        
        public Result Then(Action<T> action)
        {
            if (IsSuccess) action(_value);
            return (Result)this;
        }

        public Result<(T, T1)> Accumulate<T1>(Func<Result<T1>> a)
        {
            if (!IsSuccess) return new Result<(T, T1)>(Messages);
            var t1 = a();
            return t1.IsSuccess ? new Result<(T, T1)>((_value, t1._value), Messages.Combine(t1.Messages)) : new Result<(T, T1)>(Messages.Combine(t1.Messages));
        }
        
        public Result<(T, T1, T2)> Accumulate<T1, T2>(Func<Result<T1>> a, Func<Result<T2>> b)
        {
            if (!IsSuccess) return new Result<(T, T1, T2)>(Messages);
            var t1 = a();
            if (!t1.IsSuccess) return new Result<(T, T1, T2)>(Messages.Combine(t1.Messages));
            var t2 = b();
            return t2.IsSuccess
                       ? new Result<(T, T1, T2)>((_value, t1._value, t2._value), Messages.Combine(t1.Messages, t2.Messages))
                       : new Result<(T, T1, T2)>(Messages.Combine(t1.Messages, t2.Messages));
        }
        
        public Result<(T, T1, T2, T3)> Accumulate<T1, T2, T3>(Func<Result<T1>> a, Func<Result<T2>> b, Func<Result<T3>> c)
        {
            if (!IsSuccess) return new Result<(T, T1, T2, T3)>(Messages);
            var t1 = a();
            if (!t1.IsSuccess) return new Result<(T, T1, T2, T3)>(Messages.Combine(t1.Messages));
            var t2 = b();
            if (!t1.IsSuccess) return new Result<(T, T1, T2, T3)>(Messages.Combine(t1.Messages, t2.Messages));
            var t3 = c();
            return t2.IsSuccess
                       ? new Result<(T, T1, T2, T3)>((_value, t1._value, t2._value, t3._value), Messages.Combine(t1.Messages, t2.Messages, t3.Messages))
                       : new Result<(T, T1, T2, T3)>(Messages.Combine(t1.Messages, t2.Messages, t3.Messages));
        }
        
        public Result<TResult> Map<TResult>(Func<T, TResult> mapFunc) => IsSuccess ? Merge(new Result<TResult>(mapFunc(_value))) : new Result<TResult>(Messages);
        public Result Bind(Func<T, Result> action) => IsSuccess ? new Result(Messages.Combine(action(_value).Messages)) : new Result(Messages);
        public Result<TResult> Bind<TResult>(Func<T, Result<TResult>> bindFunc)  => IsSuccess ? Merge(bindFunc(_value)) : new Result<TResult>(Messages);
        public Result<T> Check(Func<T, Result> checkFunc) => IsSuccess ? new Result<T>(_value, Messages.Combine(checkFunc(_value).Messages)) : this;

        public Result<T> Assert(Predicate<T> assertPredicate, string exceptionErrorMessageIfFalse)
        {
            if (IsSuccess && !assertPredicate(_value)) throw new InternalCompilerException(exceptionErrorMessageIfFalse);
            return this;
        }
        
        public Result<TResult> Cast<TResult>(Context context, string? contextString = null)
        {
            if (!IsSuccess) return new Result<TResult>(Messages);
            if (_value is TResult casted) return new Result<TResult>(casted, Messages);
            var msg = context.Trace(MessageCode.InvalidCast, contextString ?? $"'{_value}' failed to cast to '{typeof(TResult)}'");
            return msg != null ? new Result<TResult>(Messages.Append(msg).ToArray()) : new Result<TResult>(Messages);
        }

        public Result<T> Else(Func<Result<T>> elseFunc)
        {
            if (IsSuccess) return this;
            var alternative = elseFunc();
            return alternative.IsSuccess ? alternative : Merge(in alternative);
        }

        public Result<T> ElseIf(bool condition, Func<Result<T>> elseFunc) => IsSuccess || !condition ? this : Else(elseFunc);

        public T ResultOr(T alternative) => IsSuccess ? _value : alternative;

        private Result<TResult> Merge<TResult>(in Result<TResult> newResult) => new Result<TResult>(newResult._value, Messages.Combine(newResult.Messages));
        
        public bool Equals(Result<T> other) => IsSuccess && EqualityComparer<T>.Default.Equals(_value, other._value);

        public override bool Equals(object obj) =>
            !ReferenceEquals(null, obj)
            && obj.GetType() == GetType()
            && Equals((Result<T>) obj);

        public override int GetHashCode() => _value == null ? 0 : EqualityComparer<T>.Default.GetHashCode(_value);
    }
}