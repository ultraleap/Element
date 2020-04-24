using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
	internal class Boolean : PreludeFixture
	{
		[
			TestCase("Bool(0)", "False"),
			TestCase("Bool(1)", "True"),
			TestCase("Bool(1.2)", "True"),
			TestCase("Bool(9999)", "True"),
			TestCase("Bool(0.1)", "True"),
			TestCase("Bool(-0.1)", "False")
		]
		[
			TestCase("Bool.not(True)", "False"),
			TestCase("Bool.not(False)", "True"),
		]
		[
			TestCase("False.and(False)", "False"),
			TestCase("False.and(True)", "False"),
			TestCase("True.and(False)", "False"),
			TestCase("True.and(True)", "True")
		]
		[
			TestCase("False.or(False)", "False"),
			TestCase("False.or(True)", "True"),
			TestCase("True.or(False)", "True"),
			TestCase("True.or(True)", "True")
		]
		[
			TestCase("False.xor(False)", "False"),
			TestCase("False.xor(True)", "True"),
			TestCase("True.xor(False)", "True"),
			TestCase("True.xor(True)", "False")
		]
		[
			TestCase("0.lt(0)", "False"),
			TestCase("0.2.lt(0)", "False"),
			TestCase("-0.2.lt(0)", "True"),
			TestCase("1.lt(2)", "True")
		]
		[
			TestCase("0.gt(0)", "False"),
			TestCase("0.2.gt(0)", "True"),
			TestCase("-0.2.gt(0)", "False"),
			TestCase("2.gt(1)", "True")
		]
		[
			TestCase("0.leq(0)", "True"),
			TestCase("0.2.leq(0)", "False"),
			TestCase("-0.2.leq(0)", "True"),
			TestCase("1.leq(2)", "True")
		]
		[
			TestCase("0.geq(0)", "True"),
			TestCase("0.2.geq(0)", "True"),
			TestCase("-0.2.geq(0)", "False"),
			TestCase("2.geq(1)", "True")
		]
		[
			TestCase("0.eq(0)", "True"),
			TestCase("1.eq(0)", "False"),
			TestCase("0.1.eq(0)", "False"),
			TestCase("999.999.eq(999.999)", "True")
		]
		[
			TestCase("0.neq(0)", "False"),
			TestCase("1.neq(0)", "True"),
			TestCase("0.1.neq(0)", "True"),
			TestCase("999.999.neq(999.999)", "False")
		]
		public void Operations(string expression, string expected) => AssertApproxEqual(ValidatedCompilationInput, expression, expected);
	}
}