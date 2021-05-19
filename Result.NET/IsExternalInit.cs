namespace System.Runtime.CompilerServices
{
    // Fix to allow record type primary constructor to work with older than .NET 5.0
    // See https://stackoverflow.com/questions/64749385/predefined-type-system-runtime-compilerservices-isexternalinit-is-not-defined
    internal static class IsExternalInit {}
}