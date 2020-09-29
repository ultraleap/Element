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
			EvaluateExpectingElementError(ValidatedCompilerInput, EleMessageCode.ConstraintNotSatisfied, new FunctionEvaluation("_(i:Num):Num = returnVec2(list(Vector2(5, 10), Vector3(5, 10, 15)).at(i))", "(1)", false));
		
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
	}
}