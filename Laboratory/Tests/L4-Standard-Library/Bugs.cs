using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Bugs : StandardLibraryFixture
    {
        public static (string constructorArgs, string fName, string constant)[] RecursionErrorArgs =
        {
            ("(0)", "Bugs.wrappedLambdasOneLine", "0"),
            ("(0)", "Bugs.wrappedLambdasExplicit", "0"),
            ("(0)", "Bugs.wrappedLambdasNamed", "0"),
            ("(0)", "Bugs.adderRecursionLambdas", "10"),
            ("(0)", "Bugs.adderRecursionReturnFunction", "10"),
        };
        [Test]
        public void RecursionError([ValueSource(nameof(RecursionErrorArgs))] (string lhs, string fName, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(u:Num):Num = " + args.fName + "(u)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        [Test]
        public void ConstraintError([Values] EvaluationMode mode)
        {
            string testFunction = "_(u:Num):Num = Bugs.constraintError(u)";
            string args = "(0)";
            string expected = "0";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args, mode),
                new ExpressionEvaluation(expected, mode));
        }
    }
}