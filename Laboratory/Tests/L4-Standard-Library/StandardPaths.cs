using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class StandardPaths : StandardLibraryFixture
    {
        public static (string circleConstructorArgs, string evaluationArgs, string expected)[] CircleArgList =
        {
            ("1", "0", "Vector3(0, 1, 0)"),
            ("1", "0.125", "Vector3(2.pow(-0.5), 2.pow(-0.5), 0)"),
            ("1", "0.25", "Vector3(1, 0, 0)"),
            ("1", "0.375", "Vector3(2.pow(-0.5), 2.pow(-0.5).mul(-1), 0)"),
            ("1", "0.5", "Vector3(0, -1, 0)"),
            ("1", "0.625", "Vector3(2.pow(-0.5).mul(-1), 2.pow(-0.5).mul(-1), 0)"),
            ("1", "0.75", "Vector3(-1, 0, 0)"),
            ("1", "0.875", "Vector3(2.pow(-0.5).mul(-1), 2.pow(-0.5), 0)"),
            ("1", "1", "Vector3(0, 1, 0)"),
            ("2", "0", "Vector3(0, 2, 0)"),
            ("2", "0.25", "Vector3(2, 0, 0)"),
            ("2", "0.5", "Vector3(0, -2, 0)"),
            ("2", "0.75", "Vector3(-2, 0, 0)"),
        };
        [Test]
        public void Circle([ValueSource(nameof(CircleArgList))]
            (string circleConstructor, string circleEvaluation, string expected) args,
            [Values] EvaluationMode mode)
        {
            string testFunction = "_(a:Num, u:Num):Vector3 = StandardPaths.circle(a).at(u)";
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
            string testFunction = "_(a:Vector3, b:Vector3, u:Num):Vector3 = StandardPaths.line(a, b).at(u)";
            string testArgs = "(" + args.constructor + ", " + args.evaluation + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.expected, mode));
        }

        public static (string lhs, string rhs)[] RectangleArgs =
        {
            ("(1, 1, 0)", "Vector3(0, 0, 0)"),
            ("(1, 1, 0.25)", "Vector3(0, 1, 0)"),
            ("(1, 1, 0.5)", "Vector3(1, 1, 0)"),
            ("(1, 1, 0.75)", "Vector3(1, 0, 0)"),
            ("(1, 1, 1)", "Vector3(0, 0, 0)"),
        };
        [Test]
        public void Rectangle([ValueSource(nameof(RectangleArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(width:Num, height:Num, u:Num):Vector3 = " +
                                  "StandardPaths.rectangle(width, height).at(u)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        
        public static (string constructorArgs, string evaluationArgs, string expected)[] LissajousArgsList =
        {
            (
                "Vector2(1, 1), Vector2(1, 1), 0",
                "0", "Vector3(0, 0, 0)"
            ),
            (
                "Vector2(1, 1), Vector2(1, 1), 0",
                "0.25", "Vector3(1, 1, 0)"
            ),
            (
                "Vector2(1, 1), Vector2(1, 1), 0",
                "0.5", "Vector3(0, 0, 0)"
            ),
            (
                "Vector2(1, 1), Vector2(1, 1), 0",
                "0.75", "Vector3(-1, -1, 0)"
            ),
            (
                "Vector2(1, 1), Vector2(1, 1), 0",
                "1", "Vector3(0, 0, 0)"
            ),
            (
                "Vector2(16, 8), Vector2(2, 4), Num.pi.div(2)",
                "0", "Vector3(0, 8, 0)"
            ),
            (
                "Vector2(16, 8), Vector2(2, 4), Num.pi.div(2)",
                "0.125", "Vector3(16, -8, 0)"
            ),
            (
                "Vector2(16, 8), Vector2(2, 4), Num.pi.div(2)",
                "0.25", "Vector3(0, 8, 0)"
            ),
            (
                "Vector2(16, 8), Vector2(2, 4), Num.pi.div(2)",
                "0.375", "Vector3(-16, -8, 0)"
            ),
            (
                "Vector2(16, 8), Vector2(2, 4), Num.pi.div(2)",
                "0.5", "Vector3(0, 8, 0)"
            ),
            (
                "Vector2(16, 8), Vector2(2, 4), Num.pi.div(2)",
                "0.625", "Vector3(16, -8, 0)"
            ),
            (
                "Vector2(16, 8), Vector2(2, 4), Num.pi.div(2)",
                "0.75", "Vector3(0, 8, 0)"
            ),
            (
                "Vector2(16, 8), Vector2(2, 4), Num.pi.div(2)",
                "0.875", "Vector3(-16, -8, 0)"
            ),
        };
        [Test]
        public void Lissajous([ValueSource(nameof(LissajousArgsList))]
            (string constructor, string evaluations, string expected) args,
            [Values] EvaluationMode mode)
        {
            string testFunction = "_(r:Vector2, f:Vector2, p:Num, u:Num):Vector3 = StandardPaths.lissajous(r, f, p).at(u)";
            string testArgs = "(" + args.constructor + ", " + args.evaluations + ")";
            AssertApproxEqualRelaxed(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.expected, mode));
        }
        
        public static (string constructorArgs, string evaluationArgs, string expected)[] RoseArgsList =
        {
            (
                "1, 1, 1",
                "0", "Vector3(1, 0, 0)"
            ),
        };
        [Test]
        public void Rose([ValueSource(nameof(RoseArgsList))]
            (string constructor, string evaluations, string expected) args,
            [Values] EvaluationMode mode)
        {
            string testFunction = "_(r:Num, f:Num, k:Num, u:Num):Vector3 = StandardPaths.rose(r, f, k).at(u)";
            string testArgs = "(" + args.constructor + ", " + args.evaluations + ")";
            AssertApproxEqualRelaxed(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.expected, mode));
        }
        
    }
}
