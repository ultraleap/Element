using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IScope
    {
        public abstract Result<IValue> Index(Identifier id, CompilationContext context);
        public abstract Result<IValue> Lookup(Identifier id, CompilationContext context);
        IReadOnlyList<IValue> Members { get; }
    }
    
    public abstract class Scope : IScope
    {
        public Result<IValue> Lookup(Identifier id, CompilationContext context) =>
            Index(id, context)
                .ElseIf(Parent != null, () => Parent!.Lookup(id, context));

        public IReadOnlyList<IValue> Members => _source.Select(pair => pair.Value).ToArray();
        public abstract IScope? Parent { get; }
        
        public Result<IValue> Index(Identifier id, CompilationContext context)
        {
            var (identifier, value) = _source.FirstOrDefault(v => v.Identifier == id);
            return identifier != default
                       ? new Result<IValue>(value)
                       : context.Trace(MessageCode.IdentifierNotFound, $"'{id}' not found in '{this}'");
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
    }
}