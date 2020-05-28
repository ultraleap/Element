namespace Element.AST
{
    /// <summary>
    /// A type that accepts any function.
    /// </summary>
    public sealed class SerializableConstraint : IConstraint
    {
        private SerializableConstraint() {}
        public static SerializableConstraint Instance { get; } = new SerializableConstraint();
        public override string ToString() => "Serializable Constraint";
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => value is ISerializableValue;
    }
}