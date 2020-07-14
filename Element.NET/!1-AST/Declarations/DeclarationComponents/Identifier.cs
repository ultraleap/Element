using System;
using Lexico;

namespace Element.AST
{
    public struct Identifier : IEquatable<Identifier>
    {
        public Identifier(string value) => String = value;

        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        [field: Regex(@"_?[a-zA-Z\u0080-\uFFFF][_a-zA-Z0-9\u0080-\uFFFF]*")]
        public string String { get; private set; }
        public override string ToString() => String;
        // ReSharper disable once NonReadonlyMemberInGetHashCode
        public override int GetHashCode() => String?.GetHashCode() ?? 0;
        public bool Equals(Identifier id) => String == id.String;
        public override bool Equals(object obj) =>
            !ReferenceEquals(null, obj)
            && obj.GetType() == GetType()
            && Equals((Identifier) obj);
    }
}