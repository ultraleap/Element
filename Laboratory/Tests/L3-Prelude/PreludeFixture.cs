using System;
using Element;
using Element.NET.TestHelpers;

namespace Laboratory.Tests.L3.Prelude
{
    internal abstract class PreludeFixture : HostFixture
    {
        protected CompilerInput ValidatedCompilerInput => new CompilerInput(TestPackageRegistry,
                                                                            Array.Empty<PackageSpecifier>(),
                                                                            new[]{GetEleFile("PreludeTestCode")},
                                                                            default);
        protected CompilerInput NonValidatedCompilerInput => new CompilerInput(TestPackageRegistry,
                                                                               Array.Empty<PackageSpecifier>(),
                                                                               new[]{GetEleFile("PreludeTestCode")},
                                                                               new CompilerOptions(default, default, true));
    }
}