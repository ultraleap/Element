using System.Linq;
using Element.CLR;
using NUnit.Framework;

namespace Laboratory.Tests
{
	internal class BoundaryCases : HostFixture
	{
		public BoundaryCases(IHost host) :base(host) { }

		private static (string Name, int Arity)[] _argCount =
		{
			("sin", 1),
			("log", 2)
		};

		[Test]
		public void ArgumentCount([ValueSource(nameof(_argCount))] (string Name, int Arity) function, [Range(0, 20)] int argCount)
		{
			var hasCorrectArgCount = function.Arity == argCount;
			var context = new HostContext
			{
				MessageHandler = TestContext.WriteLine,
				ErrorHandler = error =>
				{
					if (hasCorrectArgCount)
						Assert.Fail(error);
					else
						PassIfMessageCodeFound(error, 6);
				}
			};
			
			_host.Execute(context, function.Name, Enumerable.Range(0, argCount).Select(i => (float)i).ToArray());
			
			if (!hasCorrectArgCount)
				Assert.Fail("Expected message code ELE6 but execution succeeded");
		}
	}
}