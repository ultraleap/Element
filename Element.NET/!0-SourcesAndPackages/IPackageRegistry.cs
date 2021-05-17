using ResultNET;

namespace Element
{
    /// <summary>
    /// A registry for obtaining element packages via package specifiers.
    /// </summary>
    public interface IPackageRegistry
    {
        Result<PackageInfo> LookupPackage(PackageSpecifier specifier, ITraceContext context);
    }
}