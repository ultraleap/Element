using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
	internal class Boolean : PreludeFixture
	{
		public Boolean(IHost host) : base(host) { }

		[
			TestCase("Bool(0)", "false"),
			TestCase("Bool(1)", "true"),
			TestCase("Bool(1.2)", "true"),
			TestCase("Bool(9999)", "true"),
			TestCase("Bool(0.1)", "true"),
			TestCase("Bool(-0.1)", "false")
		]
		[
			TestCase("Bool.not(true)", "false"),
			TestCase("Bool.not(false)", "true"),
		]
		[
			TestCase("false.and(false)", "false"),
			TestCase("false.and(true)", "false"),
			TestCase("true.and(false)", "false"),
			TestCase("true.and(true)", "true")
		]
		[
			TestCase("false.or(false)", "false"),
			TestCase("false.or(true)", "true"),
			TestCase("true.or(false)", "true"),
			TestCase("true.or(true)", "true")
		]
		[
			TestCase("false.xor(false)", "false"),
			TestCase("false.xor(true)", "true"),
			TestCase("true.xor(false)", "true"),
			TestCase("true.xor(true)", "false")
		]
		[
			TestCase("0.lt(0)", "false"),
			TestCase("0.2.lt(0)", "false"),
			TestCase("-0.2.lt(0)", "true"),
			TestCase("1.lt(2)", "true")
		]
		[
			TestCase("0.gt(0)", "false"),
			TestCase("0.2.gt(0)", "true"),
			TestCase("-0.2.gt(0)", "false"),
			TestCase("2.gt(1)", "true")
		]
		[
			TestCase("0.leq(0)", "true"),
			TestCase("0.2.leq(0)", "false"),
			TestCase("-0.2.leq(0)", "true"),
			TestCase("1.leq(2)", "true")
		]
		[
			TestCase("0.geq(0)", "true"),
			TestCase("0.2.geq(0)", "true"),
			TestCase("-0.2.geq(0)", "false"),
			TestCase("2.geq(1)", "true")
		]
		[
			TestCase("0.eq(0)", "true"),
			TestCase("1.eq(0)", "false"),
			TestCase("0.1.eq(0)", "false"),
			TestCase("999.999.eq(999.999)", "true")
		]
		[
			TestCase("0.neq(0)", "false"),
			TestCase("1.neq(0)", "true"),
			TestCase("0.1.neq(0)", "true"),
			TestCase("999.999.neq(999.999)", "false")
		]
		public void Operations(string expression, string expected) => AssertApproxEqual(ValidatedCompilationInput, expression, expected);
	}
}