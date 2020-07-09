using Lexico;

namespace Element.AST
{
    public struct Identifier
    {
        public Identifier(string value) => Value = value;

        [Regex(@"_?[a-zA-Z\u0080-\uFFFF][_a-zA-Z0-9\u0080-\uFFFF]*")]
        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        public string Value { get; private set; }
        public static implicit operator string(Identifier i) => i.Value;
        public override string ToString() => Value;
        // ReSharper disable once NonReadonlyMemberInGetHashCode
        public override int GetHashCode() => Value?.GetHashCode() ?? 0;
        private bool Equals(Identifier id) => Value == id.Value;
        public override bool Equals(object obj) =>
            !ReferenceEquals(null, obj)
            && obj.GetType() == GetType()
            && Equals((Identifier) obj);
    }
}