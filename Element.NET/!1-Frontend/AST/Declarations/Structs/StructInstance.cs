using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class StructInstance : ScopeBase, ISerializableValue, IReadOnlyList<IValue>
    {
        public StructDeclaration DeclaringStruct { get; }

        public override IValue this[Identifier id, bool recurse, CompilationContext compilationContext] =>
            IndexCache(id)
            ?? DeclaringStruct.ResolveInstanceFunction(id, this, compilationContext);

        public StructInstance(StructDeclaration declaringStruct, Port[] inputs, IValue[] memberValues)
        {
            DeclaringStruct = declaringStruct;
            SetRange(memberValues.WithoutDiscardedArguments(inputs));
        }

        public IValue this[int index] => IndexCache(index);
        public IEnumerable<Element.Expression> Serialize(CompilationContext context)
        {
            var serializableValues = this.Select(v => v as ISerializableValue ?? CompilationError.Instance).ToArray();
            if (serializableValues.Any(sv => sv == CompilationError.Instance))
            {
                return new []{CompilationError.Instance};
            }
            return serializableValues.SelectMany(m => m.Serialize(context));
            // TODO: List serialization path
        }
        
        /*public bool Serialize(IValue instance, ref Element.Expression[] serialized, ref int position, CompilationContext compilationContext) =>
            instance is StructInstance listInstance
            && EvaluateElements(listInstance, compilationContext).Serialize(ref serialized, ref position, compilationContext);

        public override IValue Deserialize(IEnumerable<Element.Expression> expressions, CompilationContext compilationContext) =>
            MakeList(expressions.ToArray(), compilationContext);*/

        public ISerializableValue Deserialize(Func<Element.Expression> nextValue, CompilationContext context) =>
            DeclaringStruct.CreateInstance(this.Cast<ISerializableValue>().Select(m => m.Deserialize(nextValue, context)).ToArray());
    }
}