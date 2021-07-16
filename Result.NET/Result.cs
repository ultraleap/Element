using System;
using System.Collections.Generic;
using System.Linq;

namespace ResultNET
{
    public readonly struct Result
    {
        public static Result Success { get; } = new Result(Array.Empty<ResultMessage>());
        public Result(ResultMessage? message) : this(message == null ? Array.Empty<ResultMessage>() : new []{message}) {}
        public Result(IReadOnlyCollection<ResultMessage> messages)
        {
            IsError = messages.Any(m => m.Info.Level >= MessageLevel.Error);
            Messages = messages;
        }
        
        public static implicit operator Result(ResultMessage? message) => new Result(message);

        public bool IsSuccess => !IsError;
        public bool IsError { get; } // Defaults to false, default Result constructor will be success
        public IReadOnlyCollection<ResultMessage> Messages { get; }
            
        public void Switch(Action<IReadOnlyCollection<ResultMessage>> onResult, Action<IReadOnlyCollection<ResultMessage>> onError)
        {
            if (IsSuccess) onResult(Messages);
            else onError(Messages);
        }
        public TResult Match<TResult>(Func<IReadOnlyCollection<ResultMessage>, TResult> onResult, Func<IReadOnlyCollection<ResultMessage>, TResult> onError) => IsSuccess ? onResult(Messages) : onError(Messages);
        public Result And(Action action)
        {
            if (IsSuccess) action();
            return this;
        }
        public Result And(Func<Result> action) => IsSuccess ? new Result(Messages.Combine(action().Messages)) : this;
        public Result<TResult> Map<TResult>(Func<TResult> mapFunc) => IsSuccess ? Merge(new Result<TResult>(mapFunc())) : new Result<TResult>(Messages);
        public Result<TResult> Bind<TResult>(Func<Result<TResult>> bindFunc) => IsSuccess ? Merge(bindFunc()) : new Result<TResult>(Messages);
        public Result Merge(in Result newResult) => new Result(Messages.Combine(newResult.Messages));
        public Result<TResult> Merge<TResult>(in Result<TResult> newResult) => new Result<TResult>(newResult, Messages.Combine(newResult.Messages));
    }

    public readonly struct Result<T> : IEquatable<Result<T>>
    {
        public Result(T value, IReadOnlyCollection<ResultMessage> messages)
        {
            IsSuccess = value != null && !messages.Any(m => m.Info.Level >= MessageLevel.Error);
            _value = value;
            Messages = messages;
        }
        
        public Result(T value) : this(value, Array.Empty<ResultMessage>()) { }
        public Result(T value, ResultMessage? message) :this(value, message == null ? Array.Empty<ResultMessage>() : new []{message}) {}
        public Result(Result<T> value, ResultMessage? message) :this(value._value, message == null ? value.Messages : value.Messages.Append(message).ToArray()) { }
        public Result(Result<T> value, IReadOnlyCollection<ResultMessage> messages) :this(value._value, messages) { }
        public Result(ResultMessage? message) : this(message == null ? Array.Empty<ResultMessage>() : new []{message}) { }
        public Result(IReadOnlyCollection<ResultMessage> messages)
        {
            IsSuccess = false; // No value was passed, a result with no messages is an error whether there are any error messages or not
            _value = default!; // Might assign null if T is reference type but there's no way to retrieve it accidentally so it's ok!
            Messages = messages;
        }
        
        public static explicit operator Result(Result<T> result) => new Result(result.Messages);
        public static implicit operator Result<T>(T v) => new Result<T>(v);
        public static implicit operator Result<T>(ResultMessage? message) => new Result<T>(message);

        public bool IsSuccess { get; } // Defaults to false, default constructor is creates an error Result<T>
        public bool IsError => !IsSuccess;
        public IReadOnlyCollection<ResultMessage> Messages { get; }
        private readonly T _value;

        public bool TryGetValue(out T value)
        {
            value = _value;
            return IsSuccess;
        }
        
        public void Switch(Action<T, IReadOnlyCollection<ResultMessage>> onResult, Action<IReadOnlyCollection<ResultMessage>> onError)
        {
            if (IsSuccess) onResult(_value, Messages);
            else onError(Messages);
        }

        public TResult Match<TResult>(Func<T, IReadOnlyCollection<ResultMessage>, TResult> onResult, Func<IReadOnlyCollection<ResultMessage>, TResult> onError) => IsSuccess ? onResult(_value, Messages) : onError(Messages);

        public Result<TResult> Branch<TResult>(Func<T, TResult> success, Func<TResult> error) => IsSuccess ? Merge(new Result<TResult>(success(_value))) : error();
        public Result<TResult> Branch<TResult>(Func<T, Result<TResult>> success, Func<Result<TResult>> error) => IsSuccess ? Merge(success(_value)) : error();
        
        public Result<T> And(Action<T> action)
        {
            if (IsSuccess) action(_value);
            return this;
        }

        public Result<T> Then(Func<T, Result> action) => IsSuccess ? new Result<T>(_value, Messages.Combine(action(_value).Messages)) : this;
        public Result<T> Then(Func<T, Result<T>> action) => IsSuccess ? Merge(action(_value)) : this;

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
            if (IsSuccess && !assertPredicate(_value)) throw new Exception(exceptionErrorMessageIfFalse);
            return this;
        }

        public Result<TResult> Cast<TResult>()
        {
            if (!IsSuccess) return new Result<TResult>(Messages);
            if (_value is TResult casted) return new Result<TResult>(casted, Messages);
            throw new InvalidCastException($"'{_value}' failed to cast to '{typeof(TResult)}'");
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