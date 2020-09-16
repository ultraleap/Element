using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public class ResolvedBlock : Value, IScope
    {
        private readonly Func<IScope, Identifier, Context, Result<IValue>> _indexFunc;
        private readonly Dictionary<Identifier, Result<IValue>> _resultCache;
      
        public ResolvedBlock(IReadOnlyList<Identifier> allMembers,
                             IEnumerable<(Identifier Identifier, IValue Value)> resolvedValues,
                             Func<IScope, Identifier, Context, Result<IValue>> indexFunc,
                             IScope? parent)
        {
            Members = allMembers;
            _indexFunc = indexFunc;
            Parent = parent;
            _resultCache = resolvedValues.ToDictionary(t => t.Identifier, t => new Result<IValue>(t.Value));
        }
        
        public ResolvedBlock(IReadOnlyList<(Identifier Identifier, IValue Value)> resolvedValues, IScope? parent)
            :this(resolvedValues.Select(m => m.Identifier).ToArray(),
                  resolvedValues,
                  (resolvedBlock, id, context) =>
                  {
                      var (foundId, value) = resolvedValues.FirstOrDefault(m => m.Identifier.Equals(id));
                      return !foundId.Equals(default)
                                 ? new Result<IValue>(value)
                                 : context.Trace(EleMessageCode.IdentifierNotFound, $"'{id}' not found when indexing {resolvedBlock}");
                  }, parent)
        { }

        public override Result<IValue> Index(Identifier id, Context context) => 
            _resultCache.TryGetValue(id, out var result)
                ? result
                : _resultCache[id] = _indexFunc(this, id, context); // Cache result from calling the index function

        public Result<IValue> Lookup(Identifier id, Context context) =>
            Index(id, context)
                .ElseIf(Parent != null, () => Parent!.Lookup(id, context));

        public override IReadOnlyList<Identifier> Members { get; }
        private IScope? Parent { get; }

        public override void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context)
        {
            foreach (var member in Members)
            {
                resultBuilder.Append(Index(member, context)
                                         .Then(v => v.Serialize(resultBuilder, context)));
            }
        }

        public override Result<IValue> Deserialize(Func<Instruction> nextValue, Context context) =>
            DeserializeMembers(nextValue, context)
                .Map(memberValues => (IValue)new ResolvedBlock(memberValues.Zip(Members, (value, identifier) => (identifier, value)).ToArray(), null));

        public Result<IEnumerable<IValue>> DeserializeMembers(Func<Instruction> nextValue, Context context) =>
            Members.Select(m => Index(m, context))
                   .BindEnumerable(resolvedMembers => resolvedMembers.Select(m => m.Deserialize(nextValue, context)).ToResultEnumerable());
    }
}