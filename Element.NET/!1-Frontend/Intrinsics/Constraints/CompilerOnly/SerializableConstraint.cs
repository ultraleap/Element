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
        Result<bool> IConstraint.MatchesConstraint(IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v=> v is ISerializableValue);
    }
}