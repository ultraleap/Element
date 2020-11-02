using System;
using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class StandardPaths : StandardLibraryFixture
    {
        public static (string circleConstructorArgs, string evaluationArgs, string expected)[] CircleArgList =
        {
            ("1, Vector3.zero", "0", "Vector3(0, 1, 0)"),
            ("1, Vector3.zero", "0.125", "Vector3(2.pow(-0.5), 2.pow(-0.5), 0)"),
            ("1, Vector3.zero", "0.25", "Vector3(1, 0, 0)"),
            ("1, Vector3.zero", "0.375", "Vector3(2.pow(-0.5), 2.pow(-0.5).mul(-1), 0)"),
            ("1, Vector3.zero", "0.5", "Vector3(0, -1, 0)"),
            ("1, Vector3.zero", "0.625", "Vector3(2.pow(-0.5).mul(-1), 2.pow(-0.5).mul(-1), 0)"),
            ("1, Vector3.zero", "0.75", "Vector3(-1, 0, 0)"),
            ("1, Vector3.zero", "0.875", "Vector3(2.pow(-0.5).mul(-1), 2.pow(-0.5), 0)"),
            ("1, Vector3.zero", "1", "Vector3(0, 1, 0)"),
            ("2, Vector3(0, 2, 1)", "0", "Vector3(0, 4, 1)"),
            ("2, Vector3(0, 2, 1)", "0.25", "Vector3(2, 2, 1)"),
            ("2, Vector3(0, 2, 1)", "0.5", "Vector3(0, 0, 1)"),
            ("2, Vector3(0, 2, 1)", "0.75", "Vector3(-2, 2, 1)"),
        };
        [Test]
        public void Circle([ValueSource(nameof(CircleArgList))]
            (string circleConstructor, string circleEvaluation, string expected) args,
            [Values] EvaluationMode mode)
        {
            string testFunction = "_(a:Num, b:Vector3, u:Num):Vector3 = StandardPaths.circle(a, b)(u)";
            string testArgs = "(" + args.circleConstructor + ", " + args.circleEvaluation + ")";
            AssertApproxEqualRelaxed(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.expected, mode));
        }

    }
}
