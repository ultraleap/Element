using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class IntrinsicType : IIntrinsic, IFunction, IType
    {
        protected abstract IntrinsicType _instance { get; }
        IType IValue.Type => TypeType.Instance;
        public virtual bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == _instance;
        public virtual IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            compilationContext.GetIntrinsicsDeclaration<DeclaredStruct>(this)
                              ?.CreateInstance(arguments, _instance);
        string IIntrinsic.Location => Name;
        public abstract string Name { get; }
        public abstract Port[] Inputs { get; }
        Port IFunctionSignature.Output => Port.ReturnPort(_instance);
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => compilationContext.GetIntrinsicsDeclaration<IntrinsicStructDeclaration>(_instance);
    }

    public abstract class SerializableIntrinsicType : IntrinsicType, ISerializableType
    {
        public abstract int Size(IValue instance, CompilationContext compilationContext);

        public abstract bool Serialize(IValue instance, ref Element.Expression[] serialized, ref int position, CompilationContext compilationContext);

        public virtual IValue Deserialize(IEnumerable<Element.Expression> expressions, CompilationContext compilationContext) => Call(expressions.ToArray(), compilationContext);
    }
}