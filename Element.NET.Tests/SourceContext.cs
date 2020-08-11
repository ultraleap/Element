using System;
using System.IO;
using System.Linq;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class SourceContextFixture : FixtureBase
    {
        [Test]
        public void IgnoresLoadingFilesWithinPackageDirectories()
        {
            var src = SourceContext.CreateAndLoad(new CompilerInput(TestPackageRegistry,
                                                                    Array.Empty<PackageSpecifier>(),
                                                                    new DirectoryInfo(Directory.GetCurrentDirectory())
                                                                        .GetFiles("*.ele", SearchOption.AllDirectories).ToArray(),
                                                                    default));
            ExpectingSuccess(src.Messages, src.IsSuccess);
        }
    }
}