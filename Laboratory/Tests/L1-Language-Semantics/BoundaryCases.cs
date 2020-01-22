using System;
using System.Linq;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
	internal class BoundaryCases : HostFixture
	{
		public BoundaryCases(Func<IHost> hostGenerator) :base(hostGenerator) { }

		private static (string Name, int Arity)[] _argCount =
		{
			("sin", 1),
			("log", 2)
		};

		[Test]
		public void ArgumentCount([ValueSource(nameof(_argCount))] (string Name, int Arity) function, [Range(0, 20)] int argCount)
		{
			var hasCorrectArgCount = function.Arity == argCount;
			var input = new CompilationInput(hasCorrectArgCount ? FailOnError : ExpectMessageCode(6));

			HostGenerator().Execute(input, function.Name, Enumerable.Range(0, argCount).Select(i => (float)i).ToArray());
			
			if (!hasCorrectArgCount)
				Assert.Fail("Expected message code ELE6 but execution succeeded");
		}
	}
}