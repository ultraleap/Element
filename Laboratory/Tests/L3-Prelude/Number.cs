/*using System;
using System.Linq;
using Element;
using NUnit.Framework;

namespace Laboratory
{
	/// <summary>
	/// Tests for num and associated functions
	/// </summary>
	internal class Number : PreludeFixture
	{
		public Number(IHost host) : base(host) { }

		private static (string ElementFunction, Func<float, float> CLRFunction)[] _unaryOpTestValues =
		{
			("ln", MathF.Log),
			("sin", MathF.Sin),
			("cos", MathF.Cos),
			("tan", MathF.Tan),
			("asin", MathF.Asin),
			("acos", MathF.Acos),
			("atan", MathF.Atan),
		};

		[Test, Pairwise]
		public void UnaryMathOpRandom([ValueSource(nameof(_unaryOpTestValues))]
		                              (string ElementFunction, Func<float, float> CLRFunction) f,
		                              [Random(-1.0e6f, 1.0e6f, 20)] float a) =>
			Assert.That(Execute(f.ElementFunction, a)[0], FloatIsApproximately(f.CLRFunction(a)));

		private static (string ElementFunction, Func<float, float, float> CLRFunction)[] _binaryOpTestValues =
		{
			("add", (a, b) => a + b),
			("sub", (a, b) => a - b),
			("mul", (a, b) => a * b),
			("div", (a, b) => a / b),
			("rem", (a, b) => a % b),
			("pow", MathF.Pow),
			("min", MathF.Min),
			("max", MathF.Max),
			("log", MathF.Log),
			("atan2", MathF.Atan2),
		};

		[Test, Pairwise]
		public void BinaryMathOpRandom([ValueSource(nameof(_binaryOpTestValues))]
		                               (string ElementFunction, Func<float, float, float> CLRFunction) f,
		                               [Random(-1.0e6f, 1.0e6f, 20)] float a,
		                               [Random(-1.0e6f, 1.0e6f, 20)] float b) =>
			Assert.That(Execute(f.ElementFunction, a, b)[0], FloatIsApproximately(f.CLRFunction(a, b)));

		[
			TestCase("ln", 0f, float.NegativeInfinity),
			TestCase("ln", 1f, 0f),
			TestCase("ln", MathF.E, 1f),
		]
		public void UnaryMathOp(string function, float a, float expected) =>
			Assert.That(Execute(function, a)[0], FloatIsApproximately(expected));

		[
			// Add
			TestCase("add", 0f, 0f, 0f),
			TestCase("add", 2f, 3f, 5f),
			TestCase("add", -1.5f, -1.5f, -3f),
			TestCase("add", 1.5f, -1.5f, 0f),

			// Sub
			TestCase("sub", 0f, 0f, 0f),
			TestCase("sub", 2f, 3f, -1f),
			TestCase("sub", -1.5f, -1.5f, 0f),
			TestCase("sub", 1.5f, -1.5f, 3f),

			// Mul
			TestCase("mul", 0f, 0f, 0f),
			TestCase("mul", 10f, 0f, 0f),
			TestCase("mul", -5f, 0f, 0f),
			TestCase("mul", 2f, 3f, 6f),
			TestCase("mul", -1.5f, -1.5f, 2.25f),
			TestCase("mul", 1.5f, -1.5f, -2.25f),

			// Div
			TestCase("div", 0f, 1f, 0f),
			TestCase("div", 1f, 10f, 0.1f),
			TestCase("div", -1f, -1f, 1f),
			TestCase("div", 1f, 0.1f, 10f),
			TestCase("div", 1f, 0f, float.PositiveInfinity),

			// Rem
			TestCase("rem", 5f, 1.5f, 0.5f),
			TestCase("rem", 5f, 1.5f, 0.5f),
			TestCase("rem", -5f, -1.5f, -0.5f),
			TestCase("rem", -5f, -1.5f, -0.5f),
			TestCase("rem", 5f, 3f, 2f),
			TestCase("rem", 5f, -3f, 2f),
			TestCase("rem", -5f, 3f, -2f),
			TestCase("rem", -5f, -3f, -2f),
			TestCase("rem", 5f, 0f, float.NaN),
			TestCase("rem", 5f, -0f, float.NaN),
			TestCase("rem", -5f, 0f, float.NaN),
			TestCase("rem", -5f, -0f, float.NaN),

			// Pow
			TestCase("pow", -5f, -0f, 1f),
			TestCase("pow", -5f, 1f, -5f),
			TestCase("pow", 12f, 2f, 144f),
			TestCase("pow", 5f, -2f, 0.04f),
			TestCase("pow", 5f, 0, 1f),
			TestCase("pow", 5f, -0f, 1f),
			TestCase("pow", -5f, 0f, 1f),
			TestCase("pow", -5f, -0f, 1f),

			// Min
			TestCase("min", 5f, -2f, -2f),
			TestCase("min", -4f, 25f, -4f),
			TestCase("min", 25f, 150f, 25f),
			TestCase("min", -140f, -2f, -140f),

			// Max
			TestCase("max", 5f, -2f, 5f),
			TestCase("max", -4f, 25f, 25f),
			TestCase("max", 25f, 150f, 150f),
			TestCase("max", -140f, -2f, -2f),

			// Log
			TestCase("log", 0f, 10f, float.NegativeInfinity),
			TestCase("log", 1f, 10f, 0f),
			TestCaseSource(nameof(_logCaseData))
		]
		public void BinaryMathOps(string function, float a, float b, float expected) =>
			Assert.That(Execute(function, a, b)[0], FloatIsApproximately(expected));

		private static object[] _logCaseData = Enumerable.Range(1, 11)
		                                                 .Select(i => new object[]
		                                                 {
			                                                 "log", MathF.Pow(10f, i - 1), 10f, i - 1f
		                                                 })
		                                                 .ToArray();
	}
}*/