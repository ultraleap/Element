using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Bugs : StandardLibraryFixture
    {
        public static (string constructorArgs, string constant)[] RecursionErrorArgs =
        {
            ("(1, 1, 0)", "Vector3(1, 1, 1)"),
        };
        [Test]
        public void RecursionError([ValueSource(nameof(RecursionErrorArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(width:Num, height:Num, u:Num):Vector3 = " +
                                  "Bugs.rectangle(width, height, Vector3.one)(u)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

    }
}