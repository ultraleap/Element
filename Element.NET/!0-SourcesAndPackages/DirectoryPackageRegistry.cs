using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Element
{
    /// <summary>
    /// Package registry implemented as a filesystem directory.
    /// </summary>
    public class DirectoryPackageRegistry : IPackageRegistry
    {
        public DirectoryPackageRegistry(DirectoryInfo registryRootDirectory)
        {
            // TODO: Packages within other package directories are an error
            _packagesByName = registryRootDirectory.GetFiles($"*{PackageManifest.FileExtension}", SearchOption.AllDirectories)
                                                   .GroupBy(fi => Path.GetFileNameWithoutExtension(fi.Name))
                                                   .ToDictionary(infos => infos.Key, infos => infos.ToArray());
        }

        private readonly Dictionary<string, FileInfo[]> _packagesByName;

        public Result<PackageInfo> LookupPackage(PackageSpecifier specifier, Context context)
        {
            if (!_packagesByName.TryGetValue(specifier.Name, out var versions))
            {
                return context.Trace(MessageLevel.Error, $"No package '{specifier.Name}' found in registry");
            }

            try
            {
                return versions.Select(fileInfo => PackageManifest.ParseFromJsonString(File.ReadAllText(fileInfo.FullName), context)
                                                                  .Bind(packageManifest =>
                                                                  {
                                                                      try
                                                                      {
                                                                          return new Result<SemVer.Version>(new SemVer.Version(packageManifest.Version));
                                                                      }
                                                                      catch (Exception e)
                                                                      {
                                                                          return context.Trace(MessageLevel.Error, e.ToString());
                                                                      }
                                                                  }).Map(version => (fileInfo, version)))
                               .BindEnumerable(t =>
                               {
                                   var matchingVersionsOrdered = t
                                                                 .Where(t => specifier.VersionRange.IsSatisfied(t.version))
                                                                 .OrderByDescending(t => t.version)
                                                                 .ToArray();

                                   return matchingVersionsOrdered.Length switch
                                   {
                                       0 => context.Trace(MessageLevel.Error, $"No version of package '{specifier.Name}' matches specified range '{specifier.VersionRange}'"),
                                       _ => new Result<FileInfo>(matchingVersionsOrdered.Last().fileInfo)
                                   };
                               }).Bind(manifest => PackageInfo.FromManifestFile(manifest, context));
            }
            catch (Exception e)
            {
                return context.Trace(MessageLevel.Error, e.ToString());
            }
        }
    }
}