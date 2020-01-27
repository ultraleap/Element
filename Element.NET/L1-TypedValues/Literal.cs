namespace Element
{
    public class Literal : IValue
    {
        public Literal(string identifier, float value)
        {
            Identifier = identifier;
            Value = value;
        }

        public float Value { get; }
        public string Identifier { get; }

        public override string ToString() => Identifier;
    }
}