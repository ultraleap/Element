using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Matrix2x2 : StandardLibraryFixture
    {

        public static string Matrix2x2Constructor = "_(a:Num, b:Num, c:Num, d:Num):Matrix2x2 = Matrix2x2(a, b, c, d)";
        
        public static (string constructorArgs, string constant)[] ConstantArgList =
        {
            ("(1, 0, 0, 1)", "Matrix2x2.identity"),
        };
        [Test]
        public void Constants([ValueSource(nameof(ConstantArgList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(Matrix2x2Constructor, args.lhs, mode),
                              new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string constructorArgs, string factory, string factoryArgs)[] FactoryArgsList =
        {
            ("(1, 2, 3, 4)", "fromRows", "(Vector2(1, 2), Vector2(3, 4))"),
            ("(1, 2, 3, 4)", "fromCols", "(Vector2(1, 3), Vector2(2, 4))")
        };
        [Test]
        public void Factory([ValueSource(nameof(FactoryArgsList))] (string lhs, string factory, string rhs) args, [Values] EvaluationMode mode)
        {
            string altConstructor = "_(v1:Vector2, v2:Vector2):Matrix2x2 = Matrix2x2." + args.factory + "(v1, v2)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(Matrix2x2Constructor, args.lhs, mode),
                new FunctionEvaluation(altConstructor, args.rhs, mode));
        }
        
        public static (string constructorArgs, string factoryArgs)[] DiagonalFactoryArgsList =
        {
            ("(3, 0, 0, 5)", "(Vector2(3, 5))"),
        };
        [Test]
        public void DiagonalFactory([ValueSource(nameof(DiagonalFactoryArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string altConstructor = "_(v1:Vector2):Matrix2x2 = Matrix2x2.fromDiagonal(v1)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(Matrix2x2Constructor, args.lhs, mode),
                new FunctionEvaluation(altConstructor, args.rhs, mode));
        }

        public static (string constructorArgs, string property, string result)[] GetPropertiesArgsList =
        {
            ("(1, 2, 3, 4)", "xCol", "Vector2(1, 3)"),
            ("(1, 2, 3, 4)", "yCol", "Vector2(2, 4)"),
            ("(1, 2, 3, 4)", "xRow", "Vector2(1, 2)"),
            ("(1, 2, 3, 4)", "yRow", "Vector2(3, 4)"),
            ("(1, 2, 3, 4)", "diagonal", "Vector2(1, 4)"),
        };
        [Test]
        public void GetProperties([ValueSource(nameof(GetPropertiesArgsList))] (string lhs, string prop, string rhs) args, [Values] EvaluationMode mode)
        {
            // Tests which construct the matrix and get the named property
            string getter = "_(a:Num, b:Num, c:Num, d:Num):Vector2 = Matrix2x2(a, b, c, d)" + "." + args.prop;
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(getter, args.lhs, mode),
                              new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string constructorArgs, string result)[] DeterminantArgsList =
        {
            ("(1, 0, 0, 1)", "1"),
            ("(1, 2, 3, 4)", "-2"),
            ("(5, 1, 2, 5)", "23"),
            ("(1, 1, 2, 1)", "-1")
        };
        [Test]
        public void Determinant([ValueSource(nameof(DeterminantArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string getDeterminant = "_(a:Num, b:Num, c:Num, d:Num):Num = Matrix2x2(a, b, c, d)" + ".determinant";
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(getDeterminant, args.lhs, mode),
                              new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string constructorArgs, string resultConstructorArgs)[] TransposeArgsList =
        {
            ("(1, 2, 3, 4)", "(1, 3, 2, 4)"),
        };
        [Test]
        public void Transpose([ValueSource(nameof(TransposeArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string getTranspose = "_(a:Num, b:Num, c:Num, d:Num):Matrix2x2 = Matrix2x2(a, b, c, d)" + ".transpose";
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(getTranspose, args.lhs, mode),
                              new FunctionEvaluation(Matrix2x2Constructor, args.rhs, mode));
        }

        public static (string matrix, string vector, string output)[] VectorMulArgsList =
        {
            ("Matrix2x2.identity",
                "Vector2(-5, 6)",
                "Vector2(-5, 6)"),
            ("Matrix2x2.fromCols(Vector2(3, 1),Vector2(-2, 4))",
                "Vector2(-5, 6)",
                "Vector2(-27, 19)"),
        };
        [Test]
        public void MultiplyVector([ValueSource(nameof(VectorMulArgsList))] (string matrix, string vector, string output) args,
            [Values] EvaluationMode mode)
        {
            string matrixMultiply = "_(m:Matrix2x2, v:Vector2):Vector2 = m.vectorMul(v)";
            string matrixMultiplyArgs = "(" + args.matrix + ", " + args.vector + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(matrixMultiply, matrixMultiplyArgs, mode),
                              new ExpressionEvaluation(args.output, mode));
        }


        public static (string m1, string m2, string output)[] MatrixMulArgsList =
        {
            (
                "Matrix2x2.identity",
                "Matrix2x2.identity",
                "Matrix2x2.identity"
            ),
            (
                "Matrix2x2(1, 2, 3, 4)",
                "Matrix2x2.identity",
                "Matrix2x2(1, 2, 3, 4)"
            ),
            (
                "Matrix2x2(1, 2, 3, 4)",
                "Matrix2x2(-1, 4, -5, 3)",
                "Matrix2x2(-11, 10, -23, 24)"
            ),
        };
        [Test]
        public void MultiplyMatrix([ValueSource(nameof(MatrixMulArgsList))] (string m1, string m2, string output) args,
            [Values] EvaluationMode mode)
        {
            string matrixMultiply = "_(m1:Matrix2x2, m2:Matrix2x2):Matrix2x2 = m1.mul(m2)";
            string matrixMultiplyArgs = "(" + args.m1 + ", " + args.m2 + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(matrixMultiply, matrixMultiplyArgs, mode),
                              new ExpressionEvaluation(args.output, mode));
        }
    }
}