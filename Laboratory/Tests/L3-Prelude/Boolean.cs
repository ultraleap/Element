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
		public void Construct(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("Bool.not(True)", "False"),
			TestCase("Bool.not(False)", "True"),
		]
		public void Not(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("False.and(False)", "False"),
			TestCase("False.and(True)", "False"),
			TestCase("True.and(False)", "False"),
			TestCase("True.and(True)", "True")
		]
		public void And(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("False.or(False)", "False"),
			TestCase("False.or(True)", "True"),
			TestCase("True.or(False)", "True"),
			TestCase("True.or(True)", "True")
		]
		public void Or(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("False.xor(False)", "False"),
			TestCase("False.xor(True)", "True"),
			TestCase("True.xor(False)", "True"),
			TestCase("True.xor(True)", "False")
		]
		public void Xor(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("0.lt(0)", "False"),
			TestCase("0.2.lt(0)", "False"),
			TestCase("-0.2.lt(0)", "True"),
			TestCase("1.lt(2)", "True")
		]
		public void LessThan(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("0.gt(0)", "False"),
			TestCase("0.2.gt(0)", "True"),
			TestCase("-0.2.gt(0)", "False"),
			TestCase("2.gt(1)", "True")
		]
		public void GreaterThan(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("0.leq(0)", "True"),
			TestCase("0.2.leq(0)", "False"),
			TestCase("-0.2.leq(0)", "True"),
			TestCase("1.leq(2)", "True")
		]
		public void LessThanOrEqual(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("0.geq(0)", "True"),
			TestCase("0.2.geq(0)", "True"),
			TestCase("-0.2.geq(0)", "False"),
			TestCase("2.geq(1)", "True")
		]
		public void GreaterLessThanOrEqual(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("0.eq(0)", "True"),
			TestCase("1.eq(0)", "False"),
			TestCase("0.1.eq(0)", "False"),
			TestCase("999.999.eq(999.999)", "True")
		]
		public void Equal(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("0.neq(0)", "False"),
			TestCase("1.neq(0)", "True"),
			TestCase("0.1.neq(0)", "True"),
			TestCase("999.999.neq(999.999)", "False")
		]
		public void NotEqual(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
		
		[
			TestCase("Bool.if(True, 1, 0)", "1"),
			TestCase("Bool.if(False, 1, 0)", "0"),
		]
		public void If(string expression, string expected) => AssertApproxEqual(CompilationInput, expected, expression);
	}
}