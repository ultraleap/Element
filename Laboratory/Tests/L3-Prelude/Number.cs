using System;
using System.Globalization;
using System.Linq;
using NUnit.Framework;

namespace Laboratory.Tests
{
	/// <summary>
	/// Tests for num and associated functions
	/// </summary>
	internal class Number : PreludeFixture
	{
		private static (string ElementFunction, Func<float, float> CLRFunction)[] _unaryOpTestValues =
		{
			("Num.ln({0})", MathF.Log),
			("Num.sin({0})", MathF.Sin),
			("Num.cos({0})", MathF.Cos),
			("Num.tan({0})", MathF.Tan),
			("Num.asin({0})", MathF.Asin),
			("Num.acos({0})", MathF.Acos),
			("Num.atan({0})", MathF.Atan),
		};

		[Test, Pairwise]
		public void UnaryMathOpRandom([ValueSource(nameof(_unaryOpTestValues))]
		                              (string ElementFunction, Func<float, float> CLRFunction) functionPair,
		                              [Random(-1.0e6f, 1.0e6f, 20)] float arg0) =>
			AssertApproxEqual(ValidatedCompilationInput,
				string.Format(functionPair.ElementFunction, arg0),
				new[]{functionPair.CLRFunction(arg0)});

		private static (string ElementFunction, Func<float, float, float> CLRFunction)[] _binaryOpTestValues =
		{
			("Num.add({0}, {1})", (a, b) => a + b),
			("Num.sub({0}, {1})", (a, b) => a - b),
			("Num.mul({0}, {1})", (a, b) => a * b),
			("Num.div({0}, {1})", (a, b) => a / b),
			("Num.rem({0}, {1})", (a, b) => a % b),
			("Num.pow({0}, {1})", MathF.Pow),
			("Num.min({0}, {1})", MathF.Min),
			("Num.max({0}, {1})", MathF.Max),
			("Num.log({0}, {1})", MathF.Log),
			("Num.atan2({0}, {1})", MathF.Atan2),
		};

		[Test, Pairwise]
		public void BinaryMathOpRandom([ValueSource(nameof(_binaryOpTestValues))]
		                               (string ElementFunction, Func<float, float, float> CLRFunction) functionPair,
		                               [Random(-1.0e6f, 1.0e6f, 20)] float arg0,
		                               [Random(-1.0e6f, 1.0e6f, 20)] float arg1) =>
			AssertApproxEqual(ValidatedCompilationInput,
				string.Format(functionPair.ElementFunction, arg0, arg1),
				new[]{functionPair.CLRFunction(arg0, arg1)});
		
		[   //constants
			TestCase("Num.NaN", "Num.NaN"),
			TestCase("Num.PositiveInfinity", "Num.PositiveInfinity"),
			TestCase("Num.NegativeInfinity", "Num.NegativeInfinity"),
			TestCase("Num.pi", "3.14159265359"),
			TestCase("Num.tau", "6.28318530718"),
			TestCase("Num.e", "2.718281828459045"),
		]
		public void Constants(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilationInput, expected, expression);
		
		[   //ln
			TestCase("Num.ln(0)", "Num.NegativeInfinity"),
			TestCase("Num.ln(1)", "0"),
			TestCase("Num.ln(Num.e)", "1"),
		]	
		[   //sin
			TestCase("Num.sin(-180.radians)", "0"),
			TestCase("Num.sin(-90.radians)", "-1"),
			TestCase("Num.sin(0.radians)", "0"),
			TestCase("Num.sin(90.radians)", "1"),
			TestCase("Num.sin(180.radians)", "0"),
		]
		[   //cos
			TestCase("Num.cos(-180.radians)", "-1"),
			TestCase("Num.cos(-90.radians)", "0"),
			TestCase("Num.cos(0.radians)", "1"),
			TestCase("Num.cos(90.radians)", "0"),
			TestCase("Num.cos(180.radians)", "-1"),
		]
		[   //tan
			TestCase("Num.tan(-45.radians)", "-1"),
			TestCase("Num.tan(0.radians)", "0"),
			TestCase("Num.tan(45.radians)", "1"),
		]
		[   //asin
			TestCase("Num.asin(-1).degrees", "-90"),
			TestCase("Num.asin(0).degrees", "0"),
			TestCase("Num.asin(1).degrees", "90"),
		]
		[   //acos
			TestCase("Num.acos(-1).degrees", "180"),
			TestCase("Num.acos(0).degrees", "90"),
			TestCase("Num.acos(1).degrees", "0"),
		]
		[   //atan
			TestCase("Num.atan(-1).degrees", "-45"),
			TestCase("Num.atan(0).degrees", "0"),
			TestCase("Num.atan(1).degrees", "45"),
		]
		[   //degrees/radians
			TestCase("180.radians", "Num.pi"),
			TestCase("Num.pi.degrees", "180")
		]
		[   //sqr
			TestCase("Num.sqr(5)", "25"),
			TestCase("Num.sqr(-5)", "25")
		]
		[   //sqrt
			TestCase("Num.sqrt(25)", "5"),
			TestCase("Num.sqrt(-25)", "Num.NaN")
		]
		[   //half
			TestCase("Num.half(0)", "0"),
			TestCase("Num.half(10)", "5"),
			TestCase("Num.half(-10)", "-5")
		]
		[   //negate
			TestCase("Num.negate(0)", "0"),
			TestCase("Num.negate(1)", "-1"),
			TestCase("Num.negate(-1)", "1")
		]
		[   //negate
			TestCase("Num.roundToZero(0)", "0"),
			TestCase("Num.roundToZero(-1.5)", "-1"),
			TestCase("Num.roundToZero(-1.25)", "-1"),
			TestCase("Num.roundToZero(-0.5)", "0"),
			TestCase("Num.roundToZero(0.5)", "0"),
			TestCase("Num.roundToZero(1.25)", "1"),
			TestCase("Num.roundToZero(1.5)", "1"),
		]
		[   //sign
			TestCase("Num.sign(-1)", "-1"),
			TestCase("Num.sign(0)", "0"),
			TestCase("Num.sign(1)", "1"),
		]
		public void UnaryMathOp(string expression, string expected) => AssertApproxEqual(ValidatedCompilationInput, expected, expression);

		[   //add
			TestCase("Num.add(0, 0)", "0"),
			TestCase("Num.add(0, 1)", "1"),
			TestCase("Num.add(1, 0)", "1"),
			TestCase("Num.add(2, 3)", "5"),
			TestCase("Num.add(-1.5, -1.5)", "-3"),
			TestCase("Num.add(1.5, -1.5)", "0"),
		]
		[   //sub
			TestCase("Num.sub(0, 0)", "0"),
			TestCase("Num.sub(0, 1)", "-1"),
			TestCase("Num.sub(1, 0)", "1"),
			TestCase("Num.sub(2, 3)", "-1"),
			TestCase("Num.sub(-1.5, -1.5)", "0"),
			TestCase("Num.sub(1.5, -1.5)", "3"),
		]
		[   //mul
			TestCase("Num.mul(0, 0)", "0"),
			TestCase("Num.mul(0, 1)", "0"),
			TestCase("Num.mul(1, 0)", "0"),
			TestCase("Num.mul(-5, 0)", "0"),
			TestCase("Num.mul(2, 3)", "6"),
			TestCase("Num.mul(-1.5, -1.5)", "2.25"),
			TestCase("Num.mul(1.5, -1.5)", "-2.25"),
		]
		[   //div
			TestCase("Num.div(0, 0)", "Num.NaN"),
			TestCase("Num.div(0, 1)", "0"),
			TestCase("Num.div(1, 0)", "Num.PositiveInfinity"),
			TestCase("Num.div(1, 10)", "0.1"),
			TestCase("Num.div(-1, -1)", "1"),
			TestCase("Num.div(1, 0.1)", "10"),
		]
		[   //rem
			TestCase("Num.rem(0, 0)", "Num.NaN"),
			TestCase("Num.rem(0, 1)", "0"),
			TestCase("Num.rem(1, 0)", "Num.NaN"),
			TestCase("Num.rem(5, 1.5)", "0.5"),
			TestCase("Num.rem(5, -1.5)", "0.5"),
			TestCase("Num.rem(-5, 1.5)", "-0.5"),
			TestCase("Num.rem(-5, -1.5)", "-0.5"),
			TestCase("Num.rem(5, 3)", "2"),
			TestCase("Num.rem(5, -3)", "2"),
			TestCase("Num.rem(-5, 3)", "-2"),
			TestCase("Num.rem(-5, -3)", "-2"),
			TestCase("Num.rem(5, 0)", "Num.NaN"),
			TestCase("Num.rem(5, -0)", "Num.NaN"),
			TestCase("Num.rem(-5, 0)", "Num.NaN"),
			TestCase("Num.rem(-5, -0)", "Num.NaN"),
		]
		[   //pow
			TestCase("Num.pow(0, 0)", "1"),
			TestCase("Num.pow(0, 1)", "0"),
			TestCase("Num.pow(1, 0)", "1"),
			TestCase("Num.pow(-5, -0)", "1"),
			TestCase("Num.pow(-5, 1)", "-5"),
			TestCase("Num.pow(12, 2)", "144"),
			TestCase("Num.pow(5, -2)", "0.04"),
			TestCase("Num.pow(5, 0)", "1"),
			TestCase("Num.pow(5, -0)", "1"),
			TestCase("Num.pow(-5, 0)", "1"),
			TestCase("Num.pow(-5, -0)", "1"),
		]
		[   //min
			TestCase("Num.min(0, 0)", "0"),
			TestCase("Num.min(0, 1)", "0"),
			TestCase("Num.min(1, 0)", "0"),
			TestCase("Num.min(5, -2)", "-2"),
			TestCase("Num.min(-4, 25)", "-4"),
			TestCase("Num.min(25, 150)", "25"),
			TestCase("Num.min(-140, -2)", "-140"),
		]
		[   //max
			TestCase("Num.max(0, 0)", "0"),
			TestCase("Num.max(0, 1)", "1"),
			TestCase("Num.max(1, 0)", "1"),
			TestCase("Num.max(5, -2)", "5"),
			TestCase("Num.max(-4, 25)", "25"),
			TestCase("Num.max(25, 150)", "150"),
			TestCase("Num.max(-140, -2)", "-2"),
		]
		[   //log
			TestCase("Num.log(0, 0)", "Num.NaN"),
			TestCase("Num.log(0, 10)", "Num.NegativeInfinity"),
			TestCase("Num.log(10, 0)", "Num.NaN"),
			TestCase("Num.log(1, 10)", "0"),
			TestCaseSource(nameof(_logCaseData))
		]
		[   //exp
			TestCase("Num.exp(0, 0)", "1"),
			TestCase("Num.exp(2, 0)", "1"),
			TestCase("Num.exp(0, 2)", "0"),
			TestCase("Num.exp(-2, 2)", "4"),
			TestCase("Num.exp(2, 2)", "4"),
			TestCase("Num.exp(1, -2)", "1"),
		]
		[   //mod
			TestCase("Num.mod(0, 0)", "Num.NaN"),
			TestCase("Num.mod(2, 0)", "Num.NaN"),
			TestCase("Num.mod(0, 2)", "0"),
			TestCase("Num.mod(5, 2)", "1"),
			TestCase("Num.mod(5, -2)", "-1"),
		]
		[   //atan2
			TestCase("Num.atan2(0, 0).degrees", "0"),
			TestCase("Num.atan2(1, 0).degrees", "90"),
			TestCase("Num.atan2(0, 1).degrees", "0"),
			TestCase("Num.atan2(-1, 0).degrees", "-90"),
			TestCase("Num.atan2(0, -1).degrees", "180"),
			TestCase("Num.atan2(0.707106769, 0.707106769).degrees", "45"),
			TestCase("Num.atan2(0.707106769, -0.707106769).degrees", "135"),
			TestCase("Num.atan2(-0.707106769, 0.707106769).degrees", "-45"),
			TestCase("Num.atan2(-0.707106769, -0.707106769).degrees", "-135"),
		]
		[   //dist
			TestCase("Num.dist(0, 10)", "10"),
			TestCase("Num.dist(0, -10)", "10"),
		]
		[   //eq
			TestCase("Num.eq(1, 1)", "true"),
			TestCase("Num.eq(0, 1)", "false"),
		]
		[   //eq
			TestCase("Num.neq(1, 1)", "false"),
			TestCase("Num.neq(0, 1)", "true"),
		]
		[   //lt
			TestCase("Num.lt(0, 1)", "true"),
			TestCase("Num.lt(1, 0)", "false"),
		]
		[   //leq
			TestCase("Num.leq(0, 1)", "true"),
			TestCase("Num.leq(1, 1)", "true"),
			TestCase("Num.leq(1, 0)", "false"),
		]
		[   //lt
			TestCase("Num.gt(0, 1)", "false"),
			TestCase("Num.gt(1, 0)", "true"),
		]
		[   //geq
			TestCase("Num.geq(0, 1)", "false"),
			TestCase("Num.geq(1, 1)", "true"),
			TestCase("Num.geq(1, 0)", "true"),
		]
		public void BinaryMathOps(string expression, string expected) => AssertApproxEqual(ValidatedCompilationInput, expected, expression);
		
		[   //lerp
			TestCase("Num.lerp(-1, 0, 1)", "0"), //not covering lerping out-of-bounds
			TestCase("Num.lerp(0, 0, 1)", "0"),
			TestCase("Num.lerp(0.5, 0, 1)", "0.5"),
			TestCase("Num.lerp(1, 0, 1)", "1"),
			TestCase("Num.lerp(2, 0, 1)", "1"), //not covering lerping out-of-bounds
		]
		[   //clamp
			TestCase("Num.clamp(5, 0, 10)", "5"),
			TestCase("Num.clamp(5, -10, 0)", "0"),
			TestCase("Num.clamp(5, 10, 20)", "10"),
			TestCaseSource(nameof(_logCaseData))
		]
		public void TernaryMathOps(string expression, string expected) => AssertApproxEqual(ValidatedCompilationInput, expected, expression);

		private static object[] _logCaseData = Enumerable.Range(1, 11)
		                                                 .Select(i => new object[]
		                                                 {
			                                                 $"Num.log({ MathF.Pow(10f, i - 1)}, 10)", (i - 1f).ToString(CultureInfo.InvariantCulture)
		                                                 })
		                                                 .ToArray();
	}
}