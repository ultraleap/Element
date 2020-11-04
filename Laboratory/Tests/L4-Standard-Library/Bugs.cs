using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Bugs : StandardLibraryFixture
    {
        public static (string constructorArgs, string fName, string constant)[] RecursionErrorArgs =
        {
            ("(0)", "Bugs.wrappedLambdasOneLine", "0"),
            ("(0)", "Bugs.wrappedLambdasExplicit", "0"),
        };
        [Test]
        public void RecursionError([ValueSource(nameof(RecursionErrorArgs))] (string lhs, string fName, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(u:Num):Num = " + args.fName + "(u)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
    }
}