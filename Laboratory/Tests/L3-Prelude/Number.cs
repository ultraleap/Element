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

		[
			TestCase("Num.ln(0)", "Num.NegativeInfinity"),
			TestCase("Num.ln(1)", "0"),
			TestCase("Num.ln(Num.e)", "1"),
		]
		public void UnaryMathOp(string function, string expected) => AssertApproxEqual(ValidatedCompilationInput, function, expected);

		[
			// Add
			TestCase("Num.add(0, 0)", "0"),
			TestCase("Num.add(2, 3)", "5"),
			TestCase("Num.add(-1.5, -1.5)", "-3"),
			TestCase("Num.add(1.5, -1.5)", "0"),

			// Sub
			TestCase("Num.sub(0, 0)", "0"),
			TestCase("Num.sub(2, 3)", "-1"),
			TestCase("Num.sub(-1.5, -1.5)", "0"),
			TestCase("Num.sub(1.5, -1.5)", "3"),

			// Mul
			TestCase("Num.mul(0, 0)", "0"),
			TestCase("Num.mul(10, 0)", "0"),
			TestCase("Num.mul(-5, 0)", "0"),
			TestCase("Num.mul(2, 3)", "6"),
			TestCase("Num.mul(-1.5, -1.5)", "2.25"),
			TestCase("Num.mul(1.5, -1.5)", "-2.25"),

			// Div
			TestCase("Num.div(0, 1)", "0"),
			TestCase("Num.div(1, 10)", "0.1"),
			TestCase("Num.div(-1, -1)", "1"),
			TestCase("Num.div(1, 0.1)", "10"),
			TestCase("Num.div(1, 0)", "Num.PositiveInfinity"),

			// Rem
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

			// Pow
			TestCase("Num.pow(-5, -0)", "1"),
			TestCase("Num.pow(-5, 1)", "-5"),
			TestCase("Num.pow(12, 2)", "144"),
			TestCase("Num.pow(5, -2)", "0.04"),
			TestCase("Num.pow(5, 0)", "1"),
			TestCase("Num.pow(5, -0)", "1"),
			TestCase("Num.pow(-5, 0)", "1"),
			TestCase("Num.pow(-5, -0)", "1"),

			// Min
			TestCase("Num.min(5, -2)", "-2"),
			TestCase("Num.min(-4, 25)", "-4"),
			TestCase("Num.min(25, 150)", "25"),
			TestCase("Num.min(-140, -2)", "-140"),

			// Max
			TestCase("Num.max(5, -2)", "5"),
			TestCase("Num.max(-4, 25)", "25"),
			TestCase("Num.max(25, 150)", "150"),
			TestCase("Num.max(-140, -2)", "-2"),

			// Log
			TestCase("Num.log(0, 10)", "Num.NegativeInfinity"),
			TestCase("Num.log(1, 10)", "0"),
			TestCaseSource(nameof(_logCaseData))
		]
		public void BinaryMathOps(string expression, string expected) => AssertApproxEqual(ValidatedCompilationInput, expression, expected);

		private static object[] _logCaseData = Enumerable.Range(1, 11)
		                                                 .Select(i => new object[]
		                                                 {
			                                                 $"Num.log({ MathF.Pow(10f, i - 1)}, 10)", (i - 1f).ToString(CultureInfo.InvariantCulture)
		                                                 })
		                                                 .ToArray();
	}
}