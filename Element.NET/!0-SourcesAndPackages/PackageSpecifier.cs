using SemVer;

namespace Element
{
    /// <summary>
    /// Specifies a version range of a specific package. 
    /// </summary>
    public readonly struct PackageSpecifier
    {
        public PackageSpecifier(string name, Range versionRange)
        {
            Name = name;
            VersionRange = versionRange;
        }

        public string Name { get; }
        public Range VersionRange { get; }

        public override string ToString() => $"{Name}-{VersionRange}";
    }
}