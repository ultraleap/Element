using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Polynomial : StandardLibraryFixture
    {
        public static (string coefficients, string x, string expected)[] ConstantArgList =
        {
            ("0", "-2", "0"),
            ("0", "0", "0"),
            ("0", "2", "0"),
            ("5", "-2", "5"),
            ("5", "0", "5"),
            ("5", "2", "5"),
        };
        [Test]
        public void Constants([ValueSource(nameof(ConstantArgList))] (string coefficients, string x, string expected) args, [Values] EvaluationMode mode)
        {
            string evalFunc = "_(c0:Num, x:Num):Num = Polynomial(list(c0)).eval(x)";
            string evalArgs = "("+ args.coefficients + "," + args.x+")";
            
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(evalFunc, evalArgs, mode),
                new ExpressionEvaluation(args.expected, mode));
        }
        
        public static (string coefficients, string x, string expected)[] OrderOneArgsList =
        {
            ("0, 0", "-2", "0"),
            ("0, 0", "0", "0"),
            ("0, 0", "2", "0"),
            ("5, 0", "-2", "5"),
            ("5, 0", "0", "5"),
            ("5, 0", "2", "5"),
            ("0, 1", "-2", "-2"),
            ("0, 1", "0", "0"),
            ("0, 1", "2", "2"),
            ("0, 2", "-2", "-4"),
            ("0, 2", "0", "0"),
            ("0, 2", "2", "4"),
            ("1, 2", "-2", "-3"),
            ("1, 2", "0", "1"),
            ("1, 2", "2", "5"),
        };
        [Test]
        public void OrderOne([ValueSource(nameof(OrderOneArgsList))] (string coefficients, string x, string expected) args, [Values] EvaluationMode mode)
        {
            string evalFunc = "_(c0:Num, c1:Num, x:Num):Num = Polynomial(list(c0, c1)).eval(x)";
            string evalArgs = "("+ args.coefficients + "," + args.x+")";
            
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(evalFunc, evalArgs, mode),
                new ExpressionEvaluation(args.expected, mode));
        }
        
        public static (string coefficients, string x, string expected)[] OrderTwoArgsList =
        {
            ("0, 0, 1", "-2", "4"),
            ("0, 0, 1", "0", "0"),
            ("0, 0, 1", "2", "4"),
            ("0, 0, 2", "-2", "8"),
            ("0, 0, 2", "0", "0"),
            ("0, 0, 2", "2", "8"),
            ("1, 2, 3", "-2", "9"),
            ("1, 2, 3", "0", "1"),
            ("1, 2, 3", "2", "17"),
        };
        [Test]
        public void OrderTwo([ValueSource(nameof(OrderTwoArgsList))] (string coefficients, string x, string expected) args, [Values] EvaluationMode mode)
        {
            string evalFunc = "_(c0:Num, c1:Num, c2:Num, x:Num):Num = Polynomial(list(c0, c1, c2)).eval(x)";
            string evalArgs = "("+ args.coefficients + "," + args.x+")";
            
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(evalFunc, evalArgs, mode),
                new ExpressionEvaluation(args.expected, mode));
        }
    }
}