using System;
using Element;
using NUnit.Framework;

namespace Laboratory
{
    [TestFixtureSource(typeof(HostArguments)), Parallelizable(ParallelScope.All)]
    internal abstract class HostFixture
    {
        protected HostFixture(Func<IHost> hostGenerator)
        {
            HostGenerator = hostGenerator;
        }

        protected readonly Func<IHost> HostGenerator;

        protected static void FailOnError(CompilerMessage message)
        {
            if (message.Level >= MessageLevel.Error)
                Assert.Fail(message.ToString());
            else
                TestContext.WriteLine(message.ToString());
        }

        protected static Action<CompilerMessage> ExpectMessageCode(int messageCode) => message =>
        {
            if (message.MessageCode == messageCode) Assert.Pass($"Received expected message code {messageCode}");
            FailOnError(message);
        };
}
}