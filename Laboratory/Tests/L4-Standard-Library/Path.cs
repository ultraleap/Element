using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Paths : StandardLibraryFixture
    {
        public static (string lhs, string rhs)[] ScaleBoundsArgs =
        {
            ("(0, 1, 3, 4, 3)", "0"),
            ("(0, 1, 3, 4, 3.5)", "0.5"),
            ("(0, 1, 3, 4, 4)", "1"),
            ("(2, 3, 3, 6, 3)", "2"),
            ("(2, 3, 3, 6, 4.5)", "2.5"),
            ("(2, 3, 3, 6, 6)", "3"),
        };
        [Test]
        public void ScaleBounds([ValueSource(nameof(ScaleBoundsArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(l1:Num, u1:Num, l2:Num, u2:Num, t:Num):Num = " +
                                  "Path.scaleInputBounds(StandardPaths.line(Vector3.zero, Vector3.one), l1, u1, l2, u2)(t).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string rhs)[] PingPongArgs =
        {
            ("0", "0"),
            ("0.5", "1"),
            ("1", "0"),
        };
        [Test]
        public void PingPong([ValueSource(nameof(PingPongArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testArgs = "(" + args.lhs+ ")";
            string testFunction = "_(u:Num):Num = Path.pingPong(StandardPaths.line(Vector3.zero, Vector3.one))(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
    }
}