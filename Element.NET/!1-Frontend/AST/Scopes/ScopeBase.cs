using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class ScopeBase : IScope
    {
        public abstract Result<IValue> this[Identifier id, bool recurse, CompilationContext context] { get; }
        
        protected Result<IValue> Index(int index) =>
            index < _source.Count
                ? new Result<IValue>(_source[index].Value)
                : (MessageCode.ArgumentOutOfRange, $"Index {index} out of range, {this} contains {_source.Count} items");
        
        protected Result<IValue> Index(Identifier id)
        {
            var (identifier, value) = _source.FirstOrDefault(v => v.Identifier == id);
            return identifier != default
                       ? new Result<IValue>(value)
                       : (MessageCode.IdentifierNotFound, $"'{id}' not found in '{this}'");
        }

        public void Validate(ResultBuilder resultBuilder, Identifier[]? identifierBlacklist = null, Identifier[]? identifierWhitelist = null)
        {
            var idHashSet = new HashSet<Identifier>();
            foreach (var (identifier, value) in _source)
            {
                identifier.Validate(resultBuilder, identifierBlacklist, identifierWhitelist);
                if (value is Declaration declaration) declaration.Validate(resultBuilder);
                if (!idHashSet.Add(identifier))
                {
                    resultBuilder.Append(MessageCode.MultipleDefinitions, $"Multiple definitions for '{identifier}'");
                }
            }
        }

        protected abstract IList<(Identifier Identifier, IValue Value)> _source { get; }
        public int Count => _source.Count;

        public IEnumerator<IValue> GetEnumerator() => _source.Select(pair => pair.Value).GetEnumerator();
        IEnumerator<IValue> IEnumerable<IValue>.GetEnumerator() => GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }
}