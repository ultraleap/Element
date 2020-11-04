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
        
        public static (string circleRadius, string expectedCircumference)[] CircleLengthArgsList =
        {
            ("2", "Num.pi.mul(4)"),
            ("10", "Num.pi.mul(20)"),
        };
        [Test]
        public void CircleLength([ValueSource(nameof(CircleLengthArgsList))]
            (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string callArgs = "(" + args.lhs + ")";
            AssertApproxEqualRelaxed(ValidatedCompilerInput,
                new FunctionEvaluation("StandardPaths.circleLength", callArgs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string constructorArgs, string evaluationArgs, string expected)[] LineArgsList =
        {
            ("Vector3.zero, Vector3(1, 0, 0)", "0", "Vector3(0, 0, 0)"),
            ("Vector3.zero, Vector3(1, 0, 0)", "0.25", "Vector3(0.25, 0, 0)"),
            ("Vector3.zero, Vector3(1, 0, 0)", "0.5", "Vector3(0.5, 0, 0)"),
            ("Vector3.zero, Vector3(1, 0, 0)", "0.75", "Vector3(0.75, 0, 0)"),
            ("Vector3.zero, Vector3(1, 0, 0)", "1", "Vector3(1, 0, 0)"),
            ("Vector3(1, 1, 1), Vector3(2, 3, 4)", "0", "Vector3(1, 1, 1)"),
            ("Vector3(1, 1, 1), Vector3(2, 3, 4)", "0.5", "Vector3(1.5, 2, 2.5)"),
            ("Vector3(1, 1, 1), Vector3(2, 3, 4)", "1", "Vector3(2, 3, 4)"),
        };
        [Test]
        public void Line([ValueSource(nameof(LineArgsList))]
            (string constructor, string evaluation, string expected) args,
            [Values] EvaluationMode mode)
        {
            string testFunction = "_(a:Vector3, b:Vector3, u:Num):Vector3 = StandardPaths.line(a, b)(u)";
            string testArgs = "(" + args.constructor + ", " + args.evaluation + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.expected, mode));
        }

        public static (string lhs, string rhs)[] RectangleArgs =
        {
            ("(1, 1, 0)", "Vector3(1, 1, 1)"),
            ("(1, 1, 0.25)", "Vector3(1, 2, 1)"),
            ("(1, 1, 0.5)", "Vector3(2, 2, 1)"),
            ("(1, 1, 0.75)", "Vector3(2, 1, 1)"),
            ("(1, 1, 1)", "Vector3(1, 1, 1)"),
        };
        [Test]
        public void Rectangle([ValueSource(nameof(RectangleArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(width:Num, height:Num, u:Num):Vector3 = " +
                                  "StandardPaths.rectangle(width, height, Vector3.one)(u)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
    }
}
