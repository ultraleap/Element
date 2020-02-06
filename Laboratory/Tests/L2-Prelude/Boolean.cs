using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
	internal class Boolean : HostFixture
	{
		public Boolean(IHost host) : base(host) { }

		private static CompilationInput CompilationInput => new CompilationInput(FailOnError);

		[
			TestCase("bool", 0f, 0f),
			TestCase("bool", 1f, 1f),
			TestCase("bool", 1.2f, 1f),
			TestCase("bool", 9999f, 1f),
			TestCase("bool", 0.1f, 1f),
			TestCase("bool", -0.1f, 0f)
		]
		[
			TestCase("not", 0f, 1f),
			TestCase("not", 1f, 0f),
		]
		public void UnaryOp(string function, float a, float expected) =>
			Assert.That(_host.Evaluate(CompilationInput, function, a)[0], Is.EqualTo(expected));

		[
			TestCase("and", 0f, 0f, 0f),
			TestCase("and", 0f, 1f, 0f),
			TestCase("and", 1f, 0f, 0f),
			TestCase("and", 1f, 1f, 1f)
		]
		[
			TestCase("or", 0f, 0f, 0f),
			TestCase("or", 0f, 1f, 1f),
			TestCase("or", 1f, 0f, 1f),
			TestCase("or", 1f, 1f, 1f)
		]
		[
			TestCase("xor", 0f, 0f, 0f),
			TestCase("xor", 0f, 1f, 1f),
			TestCase("xor", 1f, 0f, 1f),
			TestCase("xor", 1f, 1f, 0f)
		]
		[
			TestCase("lt", 0f, 0f, 0f),
			TestCase("lt", 0.2f, 0f, 0f),
			TestCase("lt", -0.2f, 0f, 1f),
			TestCase("lt", 1f, 2f, 1f)
		]
		[
			TestCase("gt", 0f, 0f, 0f),
			TestCase("gt", 0.2f, 0f, 1f),
			TestCase("gt", -0.2f, 0f, 0f),
			TestCase("gt", 2f, 1f, 1f)
		]
		[
			TestCase("leq", 0f, 0f, 1f),
			TestCase("leq", 0.2f, 0f, 0f),
			TestCase("leq", -0.2f, 0f, 1f),
			TestCase("leq", 1f, 2f, 1f)
		]
		[
			TestCase("geq", 0f, 0f, 1f),
			TestCase("geq", 0.2f, 0f, 1f),
			TestCase("geq", -0.2f, 0f, 0f),
			TestCase("geq", 2f, 1f, 1f)
		]
		[
			TestCase("eq", 0f, 0f, 1f),
			TestCase("eq", 1f, 0f, 0f),
			TestCase("eq", 0.1f, 0f, 0f),
			TestCase("eq", 999.999f, 999.999f, 1f)
		]
		[
			TestCase("neq", 0f, 0f, 0f),
			TestCase("neq", 1f, 0f, 1f),
			TestCase("neq", 0.1f, 0f, 1f),
			TestCase("neq", 999.999f, 999.999f, 0f)
		]
		public void BinaryOp(string function, float a, float b, float expected) =>
			Assert.That(_host.Evaluate(CompilationInput, function, a, b)[0], Is.EqualTo(expected));
	}
}