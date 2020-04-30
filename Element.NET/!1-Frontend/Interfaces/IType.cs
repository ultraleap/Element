using System.Collections.Generic;

namespace Element.AST
{
    public interface IType : IConstraint
    {
        string Name { get; }
    }

    public interface ISerializableType : IType
    {
        int Size(IValue instance, CompilationContext compilationContext);
        bool Serialize(IValue instance, ref Element.Expression[] serialized, ref int position, CompilationContext compilationContext);
        IValue Deserialize(IEnumerable<Element.Expression> expressions, CompilationContext compilationContext);
    }
}