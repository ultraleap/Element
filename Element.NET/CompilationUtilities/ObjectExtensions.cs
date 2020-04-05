namespace Element
{
    public static class ObjectExtensions
    {
        public static TReturn Return<TReturn>(this object _, TReturn rtn) => rtn;
    }
}