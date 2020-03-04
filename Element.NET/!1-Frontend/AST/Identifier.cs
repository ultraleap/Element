using Lexico;

namespace Element.AST
{
    public class Identifier
    {
        // ReSharper disable once UnusedMember.Global
        public Identifier() {} // Need parameterless constructor for Lexico to construct instance
        public Identifier(string value) {Value = value;}

        // https://stackoverflow.com/questions/4400348/match-c-sharp-unicode-identifier-using-regex
        [Regex(@"_?[\p{L}\p{Nl}][_\p{L}\p{Nl}\p{Mn}\p{Mc}\p{Nd}\p{Pc}\p{Cf}]*")]
        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        private string Value { get; set; }
        public static implicit operator string(Identifier i) => i.Value;
        public override string ToString() => Value;
        // ReSharper disable once NonReadonlyMemberInGetHashCode
        public override int GetHashCode() => Value.GetHashCode();
        public override bool Equals(object obj) => obj?.Equals(Value) ?? (Value == null);
    }
}