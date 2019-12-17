namespace Element.Laboratory
{
	using System.Linq;
	using NUnit.Framework;

	internal class BoundaryCases : Laboratory.CompilerFixture
	{
		public BoundaryCases(Laboratory.ICommandInterface commandInterface)
			: base(commandInterface)
		{
			
		}

		private static (string Name, int Arity)[] _argCount =
		{
			("sin", 1),
			("log", 2)
		};

		[Test]
		public void ArgumentCount([ValueSource(nameof(_argCount))] (string Name, int Arity) function, [Range(0, 20)] int argCount)
		{
			var hasCorrectArgCount = function.Arity == argCount;
			Context.MessageHandler = (messages, anyErrors) =>
			{
				Assert.AreNotEqual(hasCorrectArgCount, anyErrors);
				if (hasCorrectArgCount)
					messages.Check("ELE0006");
			};
			
			Execute(function.Name, Enumerable.Range(0, argCount).Select(i => (float)i).ToArray());
			
			if (!hasCorrectArgCount)
				Assert.Fail("Expected message code ELE0006 but execution succeeded");
		}
	}
}