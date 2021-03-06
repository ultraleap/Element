using System.Globalization;
using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
	/// <summary>
	/// Tests for List and associated functions
	/// </summary>
	internal class List : PreludeFixture
	{
		[Test]
		public void IndexListWithStructuredElements() =>
			AssertApproxEqual(ValidatedCompilerInput, "5", "List.repeat(Vector3(5, 5, 5), 3).at(2).z");
		
		[Test]
		public void IndexLiteralList() =>
			AssertApproxEqual(ValidatedCompilerInput, "Vector3(5, 5, 5)", "list(Vector3(5, 5, 5)).at(0)");
		
		[Test]
		public void IndexLiteralListWithStructuredElements() =>
			AssertApproxEqual(ValidatedCompilerInput, "5", "list(Vector3(5, 5, 5)).at(0).z");
		
		[Test]
		public void SerializeListElement() =>
			AssertApproxEqual(ValidatedCompilerInput, "Vector3(5, 5, 5)", "List.repeat(Vector3(5, 5, 5), 3).at(2)");

		[Test]
		public void CallWithListElementOfNonHomogenousList() =>
			EvaluateExpectingError(ValidatedCompilerInput, ElementMessage.ExpectedHomogenousItems, new FunctionEvaluation("_(i:Num):Num = returnVec2(list(Vector2(5, 10), Vector3(5, 10, 15)).at(i))", "(1)", false));
		
		private static readonly (float, float)[] _factorialArguments =
		{
			(0f, 1f),
			(1f, 1f),
			(2f, 2f),
			(3f, 6f),
			(4f, 24f),
			(5f, 120f),
			(6f, 720f),
			(7f, 5040f),
			(8f, 40320f),
			(9f, 362880f),
			(10f, 3628800f),
			(11f, 39916800f),
		};
		
		[TestCaseSource(nameof(_factorialArguments))]
		public void FactorialUsingForWithVector2((float factorial, float expectedResult) f) =>
			AssertApproxEqual(ValidatedCompilerInput, f.expectedResult.ToString(CultureInfo.InvariantCulture), $"factorial({f.factorial.ToString(CultureInfo.InvariantCulture)})");
		
		[TestCaseSource(nameof(_factorialArguments))]
		public void FactorialUsingForWithTupleAndLambdas((float factorial, float expectedResult) f) =>
			AssertApproxEqual(ValidatedCompilerInput, f.expectedResult.ToString(CultureInfo.InvariantCulture), $"factorialExpressionBodied({f.factorial.ToString(CultureInfo.InvariantCulture)})");
		
		private static readonly (float, float)[] _resolveDifferentReturnTypesArguments =
		{
			(-1.75f, 0f),
			(1.75f, 1f),
		};

		[TestCaseSource(nameof(_resolveDifferentReturnTypesArguments))]
		public void CheckResolveDifferentReturnTypes((float a, float expectedResult) f) 
			=> AssertApproxEqual( ValidatedCompilerInput, 
				new FunctionEvaluation("_(idx:Num):Num = resolveDifferentReturnTypes(idx).at(0)", $"({f.a.ToString(CultureInfo.InvariantCulture)})", EvaluationMode.Compiled), 
				new[] {f.expectedResult});

		private static readonly (int, string)[] _topLevelStructInstanceValueArguments =
		{
			(0, "Vector3(5, 5, 5)"),
			(1, "Vector3(10, 10, 10)"),
			(2, "Vector3(15, 15, 15)")
		};

		[Test]
		public void TopLevelStructInstancePickedFromListIsSerializable([ValueSource(nameof(_topLevelStructInstanceValueArguments))]
		                                                               (int index, string resultExpression) args,
		                                                               [Values] EvaluationMode mode) =>
			AssertApproxEqual(ValidatedCompilerInput, new FunctionEvaluation("topLevelStructFromListElements", $"({args.index})", mode), new ExpressionEvaluation(args.resultExpression, EvaluationMode.Interpreted));

		public static (string inputList, string callArgs, string component, string expected)[] EnumeratedArguments =
		{
			("list(10, 11, 12)", "(0)", "idx", "0"),
			("list(10, 11, 12)", "(0)", "val", "10"),
			("list(10, 11, 12)", "(1)", "idx", "1"),
			("list(10, 11, 12)", "(1)", "val", "11"),
			("list(10, 11, 12)", "(2)", "idx", "2"),
			("list(10, 11, 12)", "(2)", "val", "12"),
		};
		[Test]
		public void Enumerate([ValueSource(nameof(EnumeratedArguments))]
			(string inputList, string element, string component, string expected) args,
			[Values] EvaluationMode mode)
		{
			string evalFunc = "_(i:Num):Num = " + args.inputList + ".enumerate.at(i)." + args.component;
			AssertApproxEqual(ValidatedCompilerInput,
				new FunctionEvaluation(evalFunc, args.element, mode),
				new ExpressionEvaluation(args.expected, mode));
		}
	}
}