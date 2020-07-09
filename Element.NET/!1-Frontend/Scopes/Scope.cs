using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public class Scope : IScope
    {
        private readonly Func<IScope, Identifier, CompilationContext, Result<IValue>> _indexFunc;
      
        public Scope(IReadOnlyList<(Identifier Identifier, IValue Value)> source, IScope? parent)
        {
            _indexFunc = (_, id, context) =>
            {
                var (foundId, value) = source.FirstOrDefault(m => m.Identifier == id);
                return foundId != default
                           ? new Result<IValue>(value)
                           : context.Trace(MessageCode.IdentifierNotFound, $"'{id}' not found in '{this}'");
            };
            Parent = parent;
            Members = source.Select(m => m.Identifier).ToArray();
        }

        public Scope(IReadOnlyList<Identifier> members, Func<IScope, Identifier, CompilationContext, Result<IValue>> indexFunc, IScope? parent)
        {
            Members = members;
            _indexFunc = indexFunc;
            Parent = parent;
        }

        public Result<IValue> Index(Identifier id, CompilationContext context) => _indexFunc(this, id, context);
        public Result<IValue> Lookup(Identifier id, CompilationContext context) =>
            Index(id, context)
                .ElseIf(Parent != null, () => Parent!.Lookup(id, context));

        public IReadOnlyList<Identifier> Members { get; }
        private IScope? Parent { get; }

        public void Serialize(ResultBuilder<List<Element.Expression>> resultBuilder, CompilationContext context)
        {
            foreach (var member in Members)
            {
                resultBuilder.Append(Index(member, context)
                                         .Do(v => v.Serialize(resultBuilder, context)));
            }
        }

        public Result<IEnumerable<IValue>> DeserializeMembers(Func<Element.Expression> nextValue, CompilationContext context) =>
            Members.Select(m => Index(m, context))
                   .BindEnumerable(resolvedMembers => resolvedMembers.Select(m => m.Deserialize(nextValue, context)).ToResultEnumerable());
    }
}