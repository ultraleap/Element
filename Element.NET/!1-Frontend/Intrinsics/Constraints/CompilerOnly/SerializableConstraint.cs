namespace Element.AST
{
    /// <summary>
    /// A type that accepts any function.
    /// </summary>
    public sealed class SerializableConstraint : Value
    {
        private SerializableConstraint() {}
        public static SerializableConstraint Instance { get; } = new SerializableConstraint();
        public override string ToString() => "<serializable constraint>";
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) =>
            value.FullyResolveValue(context).Map(v=> v.Serialize(context).IsSuccess);
    }
}