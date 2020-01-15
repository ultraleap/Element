namespace Laboratory
{
    internal interface IHost
    {
        float[] Execute(HostContext context, string functionName, params float[] functionArgs);
    }
}