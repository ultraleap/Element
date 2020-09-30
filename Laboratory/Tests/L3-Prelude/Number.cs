using System;
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
	/// <summary>
	/// Tests for num and associated functions
	/// </summary>
	internal class Number : PreludeFixture
	{
		[
			TestCase("Num(0)", "0"),
			TestCase("Num(1)", "1"),
			TestCase("Num(1.2)", "1.2"),
			TestCase("Num(9999)", "9999"),
			TestCase("Num(0.1)", "0.1"),
			TestCase("Num(-0.1)", "-0.1")
		]
		public void Construct(string expression, string expected) => AssertApproxEqual(ValidatedCompilerInput, expected, expression);

		
		#region Random
		
		// private static (string ElementFunction, Func<float, float> CLRFunction)[] _unaryOpTestValues =
		// {
		// 	("Num.ln({0})", MathF.Log),
		// 	("Num.sin({0})", MathF.Sin),
		// 	("Num.cos({0})", MathF.Cos),
		// 	("Num.tan({0})", MathF.Tan),
		// 	("Num.asin({0})", MathF.Asin),
		// 	("Num.acos({0})", MathF.Acos),
		// 	("Num.atan({0})", MathF.Atan),
		// };
		//
		// [Test, Pairwise]
		// public void UnaryMathOpRandom([ValueSource(nameof(_unaryOpTestValues))]
		//                               (string ElementFunction, Func<float, float> CLRFunction) functionPair,
		//                               [Random(-1.0e6f, 1.0e6f, 20)] float arg0) =>
		// 	AssertApproxEqual(ValidatedCompilationInput,
		// 		string.Format(functionPair.ElementFunction, arg0),
		// 		new[]{functionPair.CLRFunction(arg0)});
		//
		// [Test, Pairwise]
		// public void UnaryMathOpRandom([ValueSource(nameof(_unaryOpTestValues))]
		//                               (string ElementFunction, Func<float, float> CLRFunction) functionPair,
		//                               [Random(-1.0e6f, 1.0e6f, 5)] float arg0) =>
		// 	AssertApproxEqual(ValidatedCompilerInput,
		// 		string.Format(functionPair.ElementFunction, arg0),
		// 		new[]{functionPair.CLRFunction(arg0)});

		private static (string ElementFunction, Func<float, float, float> CLRFunction)[] _binaryOpMap =
		{
			("Num.add({0}, {1})", (a, b) => a + b),
			("Num.sub({0}, {1})", (a, b) => a - b),
			("Num.mul({0}, {1})", (a, b) => a * b),
			("Num.div({0}, {1})", (a, b) => a / b),
			("Num.rem({0}, {1})", (a, b) => a % b),
			("Num.min({0}, {1})", MathF.Min),
			("Num.max({0}, {1})", MathF.Max),
			("Num.atan2({0}, {1})", MathF.Atan2),
		};
		[Test, Pairwise]
		public void BinaryMathOpRandom([ValueSource(nameof(_binaryOpMap))]
			(string ElementFunction, Func<float, float, float> CLRFunction) functionPair,
			[Random(-1.0e6f, 1.0e6f, 5)] float arg0,
			[Random(-1.0e6f, 1.0e6f, 5)] float arg1) =>
			AssertApproxEqual(ValidatedCompilerInput,
				string.Format(functionPair.ElementFunction, arg0, arg1),
				new[]{functionPair.CLRFunction(arg0, arg1)});
		
		private static (string ElementFunction, Func<float, float, float> CLRFunction)[] _additionalBinaryOpMap =
		{
			("Num.pow({0}, {1})", MathF.Pow),
			("Num.log({0}, {1})", MathF.Log),
		};

		[Test, Pairwise]
		public void AdditionalBinaryMathOpRandom([ValueSource(nameof(_additionalBinaryOpMap))]
		                               (string ElementFunction, Func<float, float, float> CLRFunction) functionPair,
		                               [Random(2f, 10f, 5)] float arg0,
		                               [Random(2f, 10f, 5)] float arg1) =>
			AssertApproxEqual(ValidatedCompilerInput,
				string.Format(functionPair.ElementFunction, arg0, arg1),
				new[]{functionPair.CLRFunction(arg0, arg1)});
		
		#endregion
		
		[
			TestCase("Num.NaN", "Num.NaN"),
			TestCase("Num.PositiveInfinity", "Num.PositiveInfinity"),
			TestCase("Num.NegativeInfinity", "Num.NegativeInfinity"),
			TestCase("Num.pi", "3.14159265359"),
			TestCase("Num.tau", "6.28318530718"),
			TestCase("Num.e", "2.718281828459045"),
		]
		public void Constants(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.ln(0)", "Num.NegativeInfinity"),
			TestCase("Num.ln(1)", "0"),
			TestCase("Num.ln(Num.e)", "1"),
		]	
		public void NaturalLog(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.sin(0)", "0"),
			TestCase("Num.sin(-180.radians)", "0"),
			TestCase("Num.sin(-90.radians)", "-1"),
			TestCase("Num.sin(0.radians)", "0"),
			TestCase("Num.sin(90.radians)", "1"),
			TestCase("Num.sin(180.radians)", "0"),
		]
		public void Sine(string expression, string expected) =>
			AssertApproxEqualRelaxed(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.cos(0)", "1"),
			TestCase("Num.cos(-180.radians)", "-1"),
			TestCase("Num.cos(-90.radians)", "0"),
			TestCase("Num.cos(0.radians)", "1"),
			TestCase("Num.cos(90.radians)", "0"),
			TestCase("Num.cos(180.radians)", "-1"),
		]
		public void Cosine(string expression, string expected) =>
			AssertApproxEqualRelaxed(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.tan(0)", "0"),
			TestCase("Num.tan(-45.radians)", "-1"),
			TestCase("Num.tan(0.radians)", "0"),
			TestCase("Num.tan(45.radians)", "1"),
		]
		public void Tangent(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.asin(0)", "0"),
			TestCase("Num.asin(-1).degrees", "-90"),
			TestCase("Num.asin(0).degrees", "0"),
			TestCase("Num.asin(1).degrees", "90"),
		]
		public void Arcsine(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.acos(0)", "Num.pi.div(2)"),
			TestCase("Num.acos(-1).degrees", "180"),
			TestCase("Num.acos(0).degrees", "90"),
			TestCase("Num.acos(1).degrees", "0"),
		]
		public void Arccosine(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.atan(0)", "0"),
			TestCase("Num.atan(-1).degrees", "-45"),
			TestCase("Num.atan(0).degrees", "0"),
			TestCase("Num.atan(1).degrees", "45"),
		]
		public void Arctangent(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("180.radians", "Num.pi"),
			TestCase("Num.pi.degrees", "180")
		]
		public void AngleConversions(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.sqr(5)", "25"),
			TestCase("Num.sqr(-5)", "25")
		]
		public void Square(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.sqrt(25)", "5"),
			TestCase("Num.sqrt(-25)", "Num.NaN")
		]
		public void SquareRoot(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.half(0)", "0"),
			TestCase("Num.half(10)", "5"),
			TestCase("Num.half(-10)", "-5")
		]
		public void Half(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.negate(0)", "0"),
			TestCase("Num.negate(1)", "-1"),
			TestCase("Num.negate(-1)", "1")
		]
		public void Negate(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.roundToZero(0)", "0"),
			TestCase("Num.roundToZero(-1.5)", "-1"),
			TestCase("Num.roundToZero(-1.25)", "-1"),
			TestCase("Num.roundToZero(-0.5)", "0"),
			TestCase("Num.roundToZero(0.5)", "0"),
			TestCase("Num.roundToZero(1.25)", "1"),
			TestCase("Num.roundToZero(1.5)", "1"),
		]
		public void Rounding(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[   //sign
			TestCase("Num.sign(-1)", "-1"),
			TestCase("Num.sign(0)", "0"),
			TestCase("Num.sign(1)", "1"),
		]
		public void Sign(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.add(0, 0)", "0"),
			TestCase("Num.add(0, 1)", "1"),
			TestCase("Num.add(1, 0)", "1"),
			TestCase("Num.add(2, 3)", "5"),
			TestCase("Num.add(-1.5, -1.5)", "-3"),
			TestCase("Num.add(1.5, -1.5)", "0"),
		]
		public void Add(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.sub(0, 0)", "0"),
			TestCase("Num.sub(0, 1)", "-1"),
			TestCase("Num.sub(1, 0)", "1"),
			TestCase("Num.sub(2, 3)", "-1"),
			TestCase("Num.sub(-1.5, -1.5)", "0"),
			TestCase("Num.sub(1.5, -1.5)", "3"),
		]
		public void Subtract(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.mul(0, 0)", "0"),
			TestCase("Num.mul(0, 1)", "0"),
			TestCase("Num.mul(1, 0)", "0"),
			TestCase("Num.mul(-5, 0)", "0"),
			TestCase("Num.mul(2, 3)", "6"),
			TestCase("Num.mul(-1.5, -1.5)", "2.25"),
			TestCase("Num.mul(1.5, -1.5)", "-2.25"),
		]
		public void Multiply(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.div(0, 0)", "Num.NaN"),
			TestCase("Num.div(0, 1)", "0"),
			TestCase("Num.div(1, 0)", "Num.PositiveInfinity"),
			TestCase("Num.div(1, 10)", "0.1"),
			TestCase("Num.div(-1, -1)", "1"),
			TestCase("Num.div(1, 0.1)", "10"),
		]
		public void Divide(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
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
		public void Remainder(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
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
		]
		public void Power(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[   //min
			TestCase("Num.min(0, 0)", "0"),
			TestCase("Num.min(0, 1)", "0"),
			TestCase("Num.min(1, 0)", "0"),
			TestCase("Num.min(5, -2)", "-2"),
			TestCase("Num.min(-4, 25)", "-4"),
			TestCase("Num.min(25, 150)", "25"),
			TestCase("Num.min(-140, -2)", "-140"),
		]
		public void Minimum(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.max(0, 0)", "0"),
			TestCase("Num.max(0, 1)", "1"),
			TestCase("Num.max(1, 0)", "1"),
			TestCase("Num.max(5, -2)", "5"),
			TestCase("Num.max(-4, 25)", "25"),
			TestCase("Num.max(25, 150)", "150"),
			TestCase("Num.max(-140, -2)", "-2"),
		]
		public void Maximum(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		private static object GenerateLogData(int index, int baseValue) 
			=> new object[] {$"Num.log({MathF.Pow(baseValue, index - 1)}, {baseValue})", $"{(index - 1)}"};

		private static IEnumerable<object> _logCaseData =
			Enumerable.Range(1, 10)
				.SelectMany(index => Enumerable.Range(2, 9) .Select(baseValue => GenerateLogData(index, baseValue)));
		
		[
			TestCase("Num.log(0, 0)", "Num.NaN"),
			TestCase("Num.log(0, 10)", "Num.NegativeInfinity"),
			TestCase("Num.log(10, 0)", "Num.NaN"),
			TestCaseSource(nameof(_logCaseData))
		]
		public void Log(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.exp(0, 0)", "1"),
			TestCase("Num.exp(2, 0)", "1"),
			TestCase("Num.exp(0, 2)", "0"),
			TestCase("Num.exp(-2, 2)", "4"),
			TestCase("Num.exp(2, 2)", "4"),
			TestCase("Num.exp(1, -2)", "1"),
		]
		public void Exponent(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.mod(0, 0)", "Num.NaN"),
			TestCase("Num.mod(2, 0)", "Num.NaN"),
			TestCase("Num.mod(0, 2)", "0"),
			TestCase("Num.mod(5, 2)", "1"),
			TestCase("Num.mod(5, -2)", "-1"),
		]
		public void Modulo(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.atan2(0, 0)", "0"),
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
		public void Atan2(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.dist(0, 10)", "10"),
			TestCase("Num.dist(0, -10)", "10"),
		]
		public void Distance(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.eq(1, 1)", "True"),
			TestCase("Num.eq(0, 1)", "False"),
		]
		public void Equal(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.neq(1, 1)", "False"),
			TestCase("Num.neq(0, 1)", "True"),
		]
		public void NotEqual(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.lt(0, 1)", "True"),
			TestCase("Num.lt(1, 0)", "False"),
		]
		public void LessThan(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[   //leq
			TestCase("Num.leq(0, 1)", "True"),
			TestCase("Num.leq(1, 1)", "True"),
			TestCase("Num.leq(1, 0)", "False"),
		]
		public void LessThanOrEqual(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[   //lt
			TestCase("Num.gt(0, 1)", "False"),
			TestCase("Num.gt(1, 0)", "True"),
		]
		public void GreaterThan(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[   //geq
			TestCase("Num.geq(0, 1)", "False"),
			TestCase("Num.geq(1, 1)", "True"),
			TestCase("Num.geq(1, 0)", "True"),
		]
		public void GreaterThanOrEqual(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
		
		[
			TestCase("Num.lerp(-0.25, 0, 1)", "-0.25"), //extrapolation
			TestCase("Num.lerp(0, 0, 1)", "0"),
			TestCase("Num.lerp(0.5, 0, 1)", "0.5"),
			TestCase("Num.lerp(1, 0, 1)", "1"),
			TestCase("Num.lerp(1.25, 0, 1)", "1.25"), //extrapolation
		]
		public void Lerp(string expression, string expected) => 
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);

		[   //clamp
			TestCase("Num.clamp(5, 0, 10)", "5"),
			TestCase("Num.clamp(5, -10, 0)", "0"),
			TestCase("Num.clamp(5, 10, 20)", "10"),
		]
		public void Clamp(string expression, string expected) => 
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
	}
}