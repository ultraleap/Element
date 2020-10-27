using System;
using System.Collections.Generic;
using System.Linq;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
	/// <summary>
	/// Tests for num and associated functions
	/// </summary>
	internal class Number : PreludeFixture
	{
		#region RandomizedTests
		
		private static (string ElementFunction, string CallExpression, Func<float, float> CLRFunction)[] _unaryOpTestValues =
		{
			("Num.ln", "({0})", MathF.Log),
			("Num.sin", "({0})", MathF.Sin),
			("Num.cos", "({0})", MathF.Cos),
			("Num.tan", "({0})", MathF.Tan),
			("Num.asin", "({0})", MathF.Asin),
			("Num.acos", "({0})", MathF.Acos),
			("Num.atan", "({0})", MathF.Atan),
			("Num.floor", "({0})", MathF.Floor),
			("Num.ceil", "({0})", MathF.Ceiling),
			("Num.abs", "({0})", MathF.Abs),
		};

		[Test, Pairwise]
		public void UnaryMathOpRandom([ValueSource(nameof(_unaryOpTestValues))]
		                              (string ElementFunction, string CallExpression, Func<float, float> CLRFunction) args,
		                              [Random(-1.0e6f, 1.0e6f, 5)] float arg0, [Values] EvaluationMode evaluationMode) =>
			AssertApproxEqual(ValidatedCompilerInput,
			                  new FunctionEvaluation(args.ElementFunction, string.Format(args.CallExpression, arg0), evaluationMode == EvaluationMode.Interpreted),
			                  new[] {args.CLRFunction(arg0)});

		private static (string ElementFunction, string CallExpression, Func<float, float, float> CLRFunction)[] _binaryOpMap =
		{
			("Num.add", "({0}, {1})", (a, b) => a + b),
			("Num.sub", "({0}, {1})", (a, b) => a - b),
			("Num.mul", "({0}, {1})", (a, b) => a * b),
			("Num.div", "({0}, {1})", (a, b) => a / b),
			("Num.rem", "({0}, {1})", (a, b) => a % b),
			("Num.min", "({0}, {1})", MathF.Min),
			("Num.max", "({0}, {1})", MathF.Max),
			("Num.atan2", "({0}, {1})", MathF.Atan2),
			("Num.log", "({0}, {1})", MathF.Log),
		};
		
		[Test, Pairwise]
		public void BinaryMathOpRandom([ValueSource(nameof(_binaryOpMap))] (string ElementFunction, string CallExpression, Func<float, float, float> CLRFunction) args,
		                               [Random(-1.0e6f, 1.0e6f, 5)] float arg0,
		                               [Random(-1.0e6f, 1.0e6f, 5)] float arg1,
		                               [Values] EvaluationMode evaluationMode) =>
			AssertApproxEqual(ValidatedCompilerInput,
			                  new FunctionEvaluation(args.ElementFunction, string.Format(args.CallExpression, arg0, arg1), evaluationMode == EvaluationMode.Interpreted),
			                  new[] {args.CLRFunction(arg0, arg1)});
		
		private static (string ElementFunction, string CallExpression, Func<float, float, float> CLRFunction)[] _additionalBinaryOpMap =
		{
			("Num.pow", "({0}, {1})", MathF.Pow),
			("Num.log", "({0}, {1})", MathF.Log),
		};

		[Test, Pairwise]
		public void AdditionalBinaryMathOpRandom([ValueSource(nameof(_additionalBinaryOpMap))]
		                               (string ElementFunction, string CallExpression, Func<float, float, float> CLRFunction) args,
		                               [Random(2f, 10f, 5)] float arg0,
		                               [Random(2f, 10f, 5)] float arg1,
		                               [Values] EvaluationMode evaluationMode) =>
			AssertApproxEqual(ValidatedCompilerInput,
				new FunctionEvaluation(args.ElementFunction, string.Format(args.CallExpression, arg0, arg1), evaluationMode == EvaluationMode.Interpreted),
				                       new[]{args.CLRFunction(arg0, arg1)});
		
		#endregion

		[Theory]
		public void Constants((string ConstantExpression,string ExpectedExpression) args, EvaluationMode evaluationMode) =>
			AssertApproxEqual(ValidatedCompilerInput, args, evaluationMode);
		
		[DatapointSource]
		public (string ConstantExpression, string ExpectedExpression)[] ConstantExpressionData =
		{
			("Num.NaN", "Num.NaN"),
			("Num.PositiveInfinity", "Num.PositiveInfinity"),
			("Num.NegativeInfinity", "Num.NegativeInfinity"),
			("Num.pi", "3.14159265359"),
			("Num.tau", "6.28318530718"),
			("Num.e", "2.718281828459045"),
		};

		[Theory]
		public void FunctionCalls((string FunctionExpression, string CallExpression, string ExpectedExpression) args, EvaluationMode evaluationMode) =>
			AssertApproxEqual(ValidatedCompilerInput, args, evaluationMode);
		
		[Test]
		public void UnstableTrigFunctions([ValueSource(nameof(UnstableTrigFunctionData))] (string FunctionExpression, string CallExpression, string ExpectedExpression) args, [Values] EvaluationMode evaluationMode) =>
			AssertApproxEqualRelaxed(ValidatedCompilerInput, args, evaluationMode);

		// These functions are unstable around 0 so we use the relaxed float comparer for an absolute error comparison instead of relative error comparison
		public static (string FunctionExpression, string CallExpression, string ExpectedExpression)[] UnstableTrigFunctionData =
		{
			("Num.sin", $"({ToRadians(-180f)})", "0"),
			("Num.sin", $"({ToRadians(180)})", "0"),
			
			("Num.cos", $"({ToRadians(-90)})", "0"),
			("Num.cos", $"({ToRadians(90)})", "0"),
		};

		[DatapointSource]
		public (string FunctionExpression, string CallExpression, string ExpectedExpression)[] FunctionCallData =
		{
			("_(a:Num):Num = Num(a)", "(0)", "0"),
			("_(a:Num):Num = Num(a)", "(Num.NegativeInfinity)", "Num.NegativeInfinity"),
			("_(a:Num):Num = Num(a)", "(False)", "0"),
			("_(a:Num):Num = Num(a)", "(True)", "1"),

			("Num.ln", "(0)", "Num.NegativeInfinity"),
			("Num.ln", "(1)", "0"),
			("Num.ln", "(Num.e)", "1"),
			
			("Num.sin", "(0)", "0"),
			("Num.sin", $"({ToRadians(-90)})", "-1"),
			("Num.sin", $"({ToRadians(0)})", "0"),
			("Num.sin", $"({ToRadians(90)})", "1"),
			
			("Num.cos", "(0)", "1"),
			("Num.cos", $"({ToRadians(-180)})", "-1"),
			("Num.cos", $"({ToRadians(0)})", "1"),
			("Num.cos", $"({ToRadians(180)})", "-1"),
			
			("Num.tan", "(0)", "0"),
			("Num.tan", $"({ToRadians(-45)})", "-1"),
			("Num.tan", $"({ToRadians(0)})", "0"),
			("Num.tan", $"({ToRadians(45)})", "1"),
			
			("Num.asin", "(0)", "0"),
			("Num.asin", "(-1)", $"{ToRadians(-90)}"),
			("Num.asin", "(0)", $"{ToRadians(0)}"),
			("Num.asin", "(1)", $"{ToRadians(90)}"),
			
			("Num.acos", "(0)", "Num.pi.div(2)"),
			("Num.acos", "(-1)", $"{ToRadians(180)}"),
			("Num.acos", "(0)", $"{ToRadians(90)}"),
			("Num.acos", "(1)", $"{ToRadians(0)}"),
			
			("Num.atan", "(0)", "0"),
			("Num.atan", "(-1)", $"{ToRadians(-45)}"),
			("Num.atan", "(0)", $"{ToRadians(0)}"),
			("Num.atan", "(1)", $"{ToRadians(45)}"),
			
			("Num.degrees", "(Num.pi)", "180"),
			("Num.radians", "(180)", "Num.pi"),
			
			("Num.sqr", "(5)", "25"),
			("Num.sqr", "(-5)", "25"),
			
			("Num.sqrt", "(25)", "5"),
			("Num.sqrt", "(-25)", "Num.NaN"),
			
			("Num.half", "(0)", "0"),
			("Num.half", "(10)", "5"),
			("Num.half", "(-10)", "-5"),
			
			("Num.negate", "(0)", "0"),
			("Num.negate", "(1)", "-1"),
			("Num.negate", "(-1)", "1"),
			
			("Num.roundToZero", "(0)", "0"),
			("Num.roundToZero", "(-1.5)", "-1"),
			("Num.roundToZero", "(-1.25)", "-1"),
			("Num.roundToZero", "(-0.5)", "0"),
			("Num.roundToZero", "(0.5)", "0"),
			("Num.roundToZero", "(1.25)", "1"),
			("Num.roundToZero", "(1.5)", "1"),
			
			("Num.sign", "(-1)", "-1"),
			("Num.sign", "(0)", "0"),
			("Num.sign", "(1)", "1"),
			
			("Num.add", "(0, 0)", "0"),
			("Num.add", "(0, 1)", "1"),
			("Num.add", "(1, 0)", "1"),
			("Num.add", "(2, 3)", "5"),
			("Num.add", "(-1.5, -1.5)", "-3"),
			("Num.add", "(1.5, -1.5)", "0"),
			
			("Num.sub", "(0, 0)", "0"),
			("Num.sub", "(0, 1)", "-1"),
			("Num.sub", "(1, 0)", "1"),
			("Num.sub", "(2, 3)", "-1"),
			("Num.sub", "(-1.5, -1.5)", "0"),
			("Num.sub", "(1.5, -1.5)", "3"),
			
			("Num.mul", "(0, 0)", "0"),
			("Num.mul", "(0, 1)", "0"),
			("Num.mul", "(1, 0)", "0"),
			("Num.mul", "(-5, 0)", "0"),
			("Num.mul", "(2, 3)", "6"),
			("Num.mul", "(-1.5, -1.5)", "2.25"),
			("Num.mul", "(1.5, -1.5)", "-2.25"),
			
			("Num.div", "(0, 0)", "Num.NaN"),
			("Num.div", "(0, 1)", "0"),
			("Num.div", "(1, 0)", "Num.PositiveInfinity"),
			("Num.div", "(1, 10)", "0.1"),
			("Num.div", "(-1, -1)", "1"),
			("Num.div", "(1, 0.1)", "10"),
			
			("Num.rem", "(0, 0)", "Num.NaN"),
			("Num.rem", "(0, 1)", "0"),
			("Num.rem", "(1, 0)", "Num.NaN"),
			("Num.rem", "(5, 1.5)", "0.5"),
			("Num.rem", "(5, -1.5)", "0.5"),
			("Num.rem", "(-5, 1.5)", "-0.5"),
			("Num.rem", "(-5, -1.5)", "-0.5"),
			("Num.rem", "(5, 3)", "2"),
			("Num.rem", "(5, -3)", "2"),
			("Num.rem", "(-5, 3)", "-2"),
			("Num.rem", "(-5, -3)", "-2"),
			("Num.rem", "(5, 0)", "Num.NaN"),
			("Num.rem", "(5, -0)", "Num.NaN"),
			("Num.rem", "(-5, 0)", "Num.NaN"),
			("Num.rem", "(-5, -0)", "Num.NaN"),
			
			("Num.pow", "(0, 0)", "1"),
			("Num.pow", "(0, 1)", "0"),
			("Num.pow", "(1, 0)", "1"),
			("Num.pow", "(-5, -0)", "1"),
			("Num.pow", "(-5, 1)", "-5"),
			("Num.pow", "(12, 2)", "144"),
			("Num.pow", "(5, -2)", "0.04"),
			("Num.pow", "(5, 0)", "1"),
			("Num.pow", "(5, -0)", "1"),
			("Num.pow", "(-5, 0)", "1"),
			("Num.pow", "(-5, -0)", "1"),
			
			("Num.min", "(0, 0)", "0"),
			("Num.min", "(0, 1)", "0"),
			("Num.min", "(1, 0)", "0"),
			("Num.min", "(5, -2)", "-2"),
			("Num.min", "(-4, 25)", "-4"),
			("Num.min", "(25, 150)", "25"),
			("Num.min", "(-140, -2)", "-140"),
			
			("Num.max", "(0, 0)", "0"),
			("Num.max", "(0, 1)", "1"),
			("Num.max", "(1, 0)", "1"),
			("Num.max", "(5, -2)", "5"),
			("Num.max", "(-4, 25)", "25"),
			("Num.max", "(25, 150)", "150"),
			("Num.max", "(-140, -2)", "-2"),
			("Num.max", "(-140, -2)", "-2"),
			
			("Num.log", "(0, 0)", "Num.NaN"),
			("Num.log", "(0, 10)", "Num.NegativeInfinity"),
			("Num.log", "(10, 0)", "Num.NaN"),
			
			("Num.exp", "(0, 0)", "1"),
			("Num.exp", "(2, 0)", "1"),
			("Num.exp", "(0, 2)", "0"),
			("Num.exp", "(-2, 2)", "4"),
			("Num.exp", "(2, 2)", "4"),
			("Num.exp", "(1, -2)", "1"),
			
			("Num.mod", "(0, 2)", "0"),
			("Num.mod", "(5, 2)", "1"),
			("Num.mod", "(5, -2)", "-1"),
			
			("Num.mod", "(0, 0)", "Num.NaN"),
			("Num.mod", "(2, 0)", "Num.NaN"),
			
			("Num.atan2", "(0, 0)", "0"),
			("Num.atan2", "(0, 0)", $"{ToRadians(0)}"),
			("Num.atan2", "(1, 0)", $"{ToRadians(90)}"),
			("Num.atan2", "(0, 1)", $"{ToRadians(0)}"),
			("Num.atan2", "(-1, 0)", $"{ToRadians(-90)}"),
			("Num.atan2", "(0, -1)", $"{ToRadians(180)}"),
			("Num.atan2", "(0.707106769, 0.707106769)", $"{ToRadians(45)}"),
			("Num.atan2", "(0.707106769, -0.707106769)", $"{ToRadians(135)}"),
			("Num.atan2", "(-0.707106769, 0.707106769)", $"{ToRadians(-45)}"),
			("Num.atan2", "(-0.707106769, -0.707106769)", $"{ToRadians(-135)}"),
			
			("Num.dist", "(0, 10)", "10"),
			("Num.dist", "(0, -10)", "10"),
			
			("Num.eq", "(1, 1)", "True"),
			("Num.eq", "(0, 1)", "False"),
			
			("Num.neq", "(1, 1)", "False"),
			("Num.neq", "(0, 1)", "True"),
			
			("Num.lt", "(0, 1)", "True"),
			("Num.lt", "(1, 0)", "False"),
			
			("Num.leq", "(0, 1)", "True"),
			("Num.leq", "(1, 1)", "True"),
			("Num.leq", "(1, 0)", "False"),
			
			("Num.gt", "(0, 1)", "False"),
			("Num.gt", "(1, 0)", "True"),
			
			("Num.geq", "(0, 1)", "False"),
			("Num.geq", "(1, 1)", "True"),
			("Num.geq", "(1, 0)", "True"),
			
			("Num.lerp", "(-0.25, 0, 1)", "-0.25"),
			("Num.lerp", "(0, 0, 1)", "0"),
			("Num.lerp", "(0.5, 0, 1)", "0.5"),
			("Num.lerp", "(1, 0, 1)", "1"),
			("Num.lerp", "(1.25, 0, 1)", "1.25"),
			
			("Num.clamp", "(5, 0, 10)", "5"),
			("Num.clamp", "(5, -10, 0)", "0"),
			("Num.clamp", "(5, 10, 20)", "10"),
		};

		[DatapointSource]
		public (string FunctionExpression, string CallExpression, EleMessageCode ExpectedError)[] FunctionCallErrorCases =
		{
			("_(a:Num):Num = Num(a)", "(Bool)", EleMessageCode.ConstraintNotSatisfied),
			("_(a:Num):Num = Num(a)", "(Vector3(5, 5, 5))", EleMessageCode.ConstraintNotSatisfied),
		};
		
		[Theory]
		public void ErrorCases((string FunctionExpression, string CallExpression, EleMessageCode ExpectedError) args, EvaluationMode evaluationMode) =>
			EvaluateExpectingElementError(ValidatedCompilerInput, args.ExpectedError, new FunctionEvaluation(args.FunctionExpression, args.CallExpression, evaluationMode == EvaluationMode.Interpreted));

		private static object GenerateLogData(int index, int baseValue) 
			=> new object[] {$"Num.log({MathF.Pow(baseValue, index - 1)}, {baseValue})", $"{(index - 1)}"};

		private static IEnumerable<object> _logCaseData =
			Enumerable.Range(1, 10)
				.SelectMany(index => Enumerable.Range(2, 9) .Select(baseValue => GenerateLogData(index, baseValue)));
		
		[TestCaseSource(nameof(_logCaseData))]
		public void Log(string expression, string expected) =>
			AssertApproxEqual(ValidatedCompilerInput, expected, expression);
	}
}