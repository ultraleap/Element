using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using ResultNET;

namespace Element
{
    /// <summary>
    /// An element package and all it's contents in memory.
    /// </summary>
    public class PackageInfo
    {
        public PackageInfo(string name, PackageManifest manifest, IReadOnlyCollection<SourceInfo> packageSources)
        {
            Name = name;
            Manifest = manifest;
            PackageSources = packageSources;
        }

        public string Name { get; }
        public PackageManifest Manifest { get; }
        public IReadOnlyCollection<SourceInfo> PackageSources { get; }

        public override string ToString() => $"{Name} [{Manifest.Version}]";

        public static Result<PackageInfo> FromManifestFilePath(string manifestFilePath, ITraceContext context) => FromManifestFile(new FileInfo(manifestFilePath), context);

        public static Result<PackageInfo> FromManifestFile(FileInfo manifestFile, ITraceContext context) =>
            !manifestFile.Extension.Equals(PackageManifest.FileExtension, StringComparison.OrdinalIgnoreCase)
                ? context.Trace(ElementMessage.FileAccessError, $"{manifestFile.Name} is not a valid package manifest, file extension should be {PackageManifest.FileExtension}")
                : FromDirectory(manifestFile.Directory, context);

        public static Result<PackageInfo> FromDirectory(DirectoryInfo directoryInfo, ITraceContext context)
        {
            var manifestFiles = directoryInfo.GetFiles($"*{PackageManifest.FileExtension}", SearchOption.TopDirectoryOnly);
            if (manifestFiles.Length != 1)
            {
                return context.Trace(ElementMessage.FileAccessError, "Package directory must have exactly 1 .bond file at root");
            }

            return FromStrings(Path.GetFileNameWithoutExtension(manifestFiles[0].FullName), File.ReadAllText(manifestFiles[0].FullName),
                               directoryInfo
                                   .GetFiles("*.ele", SearchOption.AllDirectories)
                                   .Select(s => (s.FullName, File.ReadAllText(s.FullName), (string?)s.Name))
                                   .ToArray(),
                               context);
        }

        public static Result<PackageInfo> FromStreams(string packageName, Stream manifestStream, IEnumerable<(string FullName, Stream Stream, string? DisplayName)> sourceStreams, ITraceContext context)
        {
            static string ReadStream(Stream stream)
            {
                using var reader = new StreamReader(stream);
                return reader.ReadToEnd();
            }

            return FromStrings(packageName, ReadStream(manifestStream), sourceStreams.Select(t => (t.FullName, ReadStream(t.Stream), t.DisplayName)).ToArray(), context);
        }

        public static Result<PackageInfo> FromStrings(string packageName, string manifestString, IEnumerable<(string FullName, string Source, string? DisplayName)> sources, ITraceContext context)
        {
            try
            {
                return PackageManifest.ParseFromJsonString(manifestString, context)
                                      .Map(manifest => new PackageInfo(packageName, manifest, sources.Select(t => new SourceInfo($"{packageName}:{t.FullName}", t.Source, t.DisplayName)).ToArray()));
            }
            catch (Exception e)
            {
                return context.Trace(MessageLevel.Error, e.ToString());
            }
        }
    }
}