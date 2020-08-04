using System.IO;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public class SourceContextFixture : FixtureBase
    {
        [Test]
        public void LoadSameFileMultipleTimesIgnoresDuplicates()
        {
            var src = SourceContext.CreateAndLoad(new CompilerInput(new CompilerSource
            {
                ExtraSourceFiles = new DirectoryInfo(Directory.GetCurrentDirectory()).GetFiles("*.ele", SearchOption.AllDirectories)
            }, new CompilerOptions()));
            ExpectingSuccess(src.Messages, src.IsSuccess);
        }
    }
}