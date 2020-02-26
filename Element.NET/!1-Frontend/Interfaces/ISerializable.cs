namespace Element.AST
{
    public interface ISerializable
    {
        int SerializedSize { get; }
        bool Serialize(ref float[] array, ref int position);
    }

    public static class SerializableExtensions
    {
        public static bool TrySerialize(this ISerializable value, out float[] serialized)
        {
            serialized = new float[value.SerializedSize];
            var position = 0;
            var success = true;
            success &= value.Serialize(ref serialized, ref position);
            return success;
        }
    }
}