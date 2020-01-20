using Element.CLR;
using NUnit.Framework;

namespace Laboratory
{
    [TestFixtureSource(typeof(HostArguments)), Parallelizable(ParallelScope.All)]
    internal abstract class HostFixture
    {
        protected HostFixture(IHost host)
        {
            _host = host;
        }

        protected readonly IHost _host;

        protected static HostContext DefaultHostContext => new HostContext
        {
            MessageHandler = TestContext.WriteLine,
            ErrorHandler = Assert.Fail
        };

        protected static void PassIfMessageCodeFound(string message, int messageCode)
        {
            if (message.ContainsMessageCode(messageCode))
                Assert.Pass(message);
            else
                TestContext.WriteLine(message);
        }
    }
}