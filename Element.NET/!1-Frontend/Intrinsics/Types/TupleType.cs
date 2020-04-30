using System.Collections.Generic;

namespace Element.AST
{
    public sealed class TupleType : SerializableIntrinsicType
    {
        public static TupleType Instance { get; } = new TupleType();
        protected override IntrinsicType _instance => Instance;
        public override string Name => "Tuple";
        public override Port[] Inputs { get; } = {Port.VariadicPort};
        
        public override int Size(IValue instance, CompilationContext compilationContext) =>
            (instance as IEnumerable<IValue>).GetSerializedSize(compilationContext);

        public override bool Serialize(IValue instance, ref Element.Expression[] serialized, ref int position, CompilationContext compilationContext) =>
            (instance as IEnumerable<IValue>).Serialize(ref serialized, ref position, compilationContext);
    }
}