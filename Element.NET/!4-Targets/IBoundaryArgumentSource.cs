namespace Element
{
    public interface IBoundaryArgumentSource
    {
        Result Set(string path, float value);
        Result<float> Get(string path);
        void ApplyArgumentChanges();
    }
}