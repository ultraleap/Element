using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class StructInstance : ScopeBase, ISerializableValue, IInstancable<StructDeclaration> //, IReadOnlyList<IValue>
    {
        public StructDeclaration DeclaringStruct { get; }

        public override Result<IValue> this[Identifier id, bool recurse, CompilationContext context] =>
            Index(id, context)
                .Else(() => DeclaringStruct.ResolveInstanceFunction(id, this, context));

        protected override IList<(Identifier Identifier, IValue Value)> _source { get; }

        public StructInstance(StructDeclaration declaringStruct, IEnumerable<Port> inputs, IEnumerable<IValue> fieldValues)
        {
            DeclaringStruct = declaringStruct;
            _source = fieldValues.WithoutDiscardedArguments(inputs).ToList();
        }

        public StructDeclaration GetDefinition(CompilationContext compilationContext)
        {
            throw new NotImplementedException();
        }

        public override string ToString() => $"Instance:{DeclaringStruct}";

        //public IValue this[int index] => IndexCache(index).mat;
        
        /*public bool Serialize(IValue instance, ref Element.Expression[] serialized, ref int position, CompilationContext compilationContext) =>
            instance is StructInstance listInstance
            && EvaluateElements(listInstance, compilationContext).Serialize(ref serialized, ref position, compilationContext);

        public override IValue Deserialize(IEnumerable<Element.Expression> expressions, CompilationContext compilationContext) =>
            MakeList(expressions.ToArray(), compilationContext);*/

        public void Serialize(ResultBuilder<IList<Element.Expression>> resultBuilder)
        {
            // TODO: List serialization
            foreach (var v in this)
            {
                if (v is ISerializableValue sv)
                {
                    sv.Serialize(resultBuilder);
                }
                else
                {
                    resultBuilder.Append(MessageCode.SerializationError, $"'{v}' is not serializable");
                }
            }
            return this.SelectMany(v => v is ISerializableValue sv
                                            ? sv.Serialize()
                                            :;
        }

        public Result<ISerializableValue> Deserialize(Func<Element.Expression> nextValue, ITrace trace) =>
            this.Select(field => field is ISerializableValue sv ? sv.Deserialize(nextValue, trace) : trace.Trace(MessageCode.SerializationError, $"'{field}' cannot be deserialized"))
                .MapEnumerable(fields => (ISerializableValue)DeclaringStruct.CreateInstance(fields));
    }
}