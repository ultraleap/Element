using System;
using System.Collections.Generic;
using System.Linq;
using ResultNET;

namespace Element.AST
{
    public class ResolvedBlock : Value, IScope
    {
        private readonly Func<IScope, Identifier, Context, Result<IValue>> _indexFunc;
        private readonly Func<IValue?>? _valueProducedFrom;
        private readonly Dictionary<Identifier, Result<IValue>> _resultCache;

        public override string SummaryString => _valueProducedFrom?.Invoke()?.SummaryString ?? base.SummaryString;
        public override string TypeOf => _valueProducedFrom?.Invoke()?.TypeOf ?? base.TypeOf;

        public ResolvedBlock(IReadOnlyList<Identifier> members,
                             IEnumerable<(Identifier Identifier, IValue Value)> alreadyResolvedMembers,
                             Func<IScope, Identifier, Context, Result<IValue>> indexFunc,
                             IScope? parent,
                             Func<IValue?>? valueProducedFrom = null)
        {
            alreadyResolvedMembers = alreadyResolvedMembers as IReadOnlyList<(Identifier Identifier, IValue Value)> ?? alreadyResolvedMembers.ToArray();
            Members = members.Concat(alreadyResolvedMembers.Select(m => m.Identifier)).Distinct().ToArray();
            _indexFunc = indexFunc;
            _valueProducedFrom = valueProducedFrom;
            Parent = parent;
            _resultCache = alreadyResolvedMembers.ToDictionary(t => t.Identifier, t => new Result<IValue>(t.Value));
        }
        
        public ResolvedBlock(IReadOnlyList<(Identifier Identifier, IValue Value)> resolvedValues,
                             IScope? parent,
                             Func<IValue?>? valueProducedFrom = null)
            :this(resolvedValues.Select(m => m.Identifier).ToArray(),
                  resolvedValues,
                  (resolvedBlock, id, context) =>
                  {
                      var (foundId, value) = resolvedValues.FirstOrDefault(m => m.Identifier.Equals(id));
                      return !foundId.Equals(default) // if we found a non-default ID then FirstOrDefault succeeded
                                 ? new Result<IValue>(value)
                                 : context.Trace(EleMessageCode.IdentifierNotFound, $"'{id}' not found when indexing {resolvedBlock}");
                  }, parent, valueProducedFrom)
        { }

        public override Result<IValue> Index(Identifier id, Context context) => 
            _resultCache.TryGetValue(id, out var result)
                ? result
                : _resultCache[id] = _indexFunc(this, id, context); // Cache result from calling the index function

        public Result<IValue> Lookup(Identifier id, Context context) => 
            Members.Any(m => m.Equals(id))
                ? Index(id, context)
                : Parent?.Lookup(id, context) ?? context.Trace(EleMessageCode.IdentifierNotFound, $"'{id}' not found when indexing {this}");

        public override IReadOnlyList<Identifier> Members { get; }
        private IScope? Parent { get; }

        public override void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context)
        {
            foreach (var member in Members)
            {
                resultBuilder.Append(Index(member, context)
                                         .And(v => v.Serialize(resultBuilder, context)));
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