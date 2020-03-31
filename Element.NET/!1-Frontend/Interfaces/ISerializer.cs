namespace Element.AST
{
    public interface ISerializer
    {
        int SerializedSize(IValue value);
        bool Serialize(IValue value, ref float[] array, ref int position);
    }
}