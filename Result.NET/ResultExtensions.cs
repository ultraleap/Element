using System;
using System.Collections.Generic;
using System.Linq;

namespace ResultNET
{
    public static class ResultExtensions
    {
        public static Result Fold(this IEnumerable<Result> results)
        {
            results = results as Result[] ?? results.ToArray();
            var messages = new ResultMessage[results.Sum(r => r.Messages.Count)];
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

        public static IReadOnlyCollection<ResultMessage> Combine(this IReadOnlyCollection<ResultMessage> a, IReadOnlyCollection<ResultMessage> b) =>
            (a.Count, b.Count) switch
            {
                (0, 0) => Array.Empty<ResultMessage>(),
                (0, _) => b,
                (_, 0) => a,
                _      => a.Concat(b).ToArray()
            };
        
        public static IReadOnlyCollection<ResultMessage> Combine(this IReadOnlyCollection<ResultMessage> a, params IReadOnlyCollection<ResultMessage>[] others) =>
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
            List<ResultMessage> messages = new List<ResultMessage>(enumerable.Messages);
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
}