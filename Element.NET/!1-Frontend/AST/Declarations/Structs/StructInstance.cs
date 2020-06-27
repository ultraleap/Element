using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class StructInstance : Value
    {
        public StructDeclaration DeclaringStruct { get; }

        private sealed class StructInstanceScope : Scope
        {
            public StructInstanceScope(IList<(Identifier Identifier, IValue Value)> source)
            {
                _source = source;
            }

            public override IScope? Parent => null;
            protected override IList<(Identifier Identifier, IValue Value)> _source { get; }
        }
        
        private readonly StructInstanceScope _scope;
        
        public StructInstance(StructDeclaration declaringStruct, IEnumerable<IValue> fieldValues)
        {
            DeclaringStruct = declaringStruct;
            _scope = new StructInstanceScope(fieldValues.WithoutDiscardedArguments(declaringStruct.Fields).ToList());
        }

        public override string ToString() => $"Instance:{DeclaringStruct}";
        
        public override Result<IValue> Index(Identifier id, CompilationContext context) =>
            _scope.Lookup(id, context).Else(() => DeclaringStruct.ResolveInstanceFunction(id, this, context));

        public override IReadOnlyList<IValue> Members => _scope.Members;

        public override void Serialize(ResultBuilder<List<Element.Expression>> resultBuilder)
        {
            // TODO: List serialization
            foreach (var member in Members)
            {
                member.Serialize(resultBuilder);
            }
        }

        public override Result<IValue> Deserialize(Func<Element.Expression> nextValue, ITrace trace) =>
            Members.Select(f => f.Deserialize(nextValue, trace))
                   .MapEnumerable(deserializedFields => (IValue) new StructInstance(DeclaringStruct, deserializedFields));
    }
}