using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Matrix2x2 : StandardLibraryFixture
    {

        public static (string constructorArgs, string constant)[] ConstantArgList =
        {
            ("(Vector2(1, 0), Vector2(0, 1))", "Matrix2x2.identity"),
        };
        [Test]
        public void Constants([ValueSource(nameof(ConstantArgList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation("Matrix2x2", args.lhs, mode),
                              new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string constructorArgs, string factory, string factoryArgs)[] FactoryArgsList =
        {
            ("(Vector2(1, 2), Vector2(3, 4))", "fromRows", "(Vector2(1, 2), Vector2(3, 4))"),
            ("(Vector2(1, 2), Vector2(3, 4))", "fromCols", "(Vector2(1, 3), Vector2(2, 4))"),
            ("(Vector2(3, 0), Vector2(0, 5))", "fromDiagonal", "(Vector2(3, 5))"),
        };
        [Test]
        public void Factory([ValueSource(nameof(FactoryArgsList))] (string lhs, string factory, string rhs) args, [Values] EvaluationMode mode)
        {
            string defaultConstructor = "Matrix2x2";
            string altConstructor = "Matrix2x2." + args.factory;
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(defaultConstructor, args.lhs, mode),
                              new FunctionEvaluation(altConstructor, args.rhs, mode));
        }

        public static (string constructorArgs, string property, string result)[] GetPropertiesArgsList =
        {
            ("(Vector2(1, 2), Vector2(3, 4))", "xCol", "Vector2(1, 3)"),
            ("(Vector2(1, 2), Vector2(3, 4))", "yCol", "Vector2(2, 4)"),
            ("(Vector2(1, 2), Vector2(3, 4))", "xRow", "Vector2(1, 2)"),
            ("(Vector2(1, 2), Vector2(3, 4))", "yRow", "Vector2(3, 4)"),
            ("(Vector2(1, 2), Vector2(3, 4))", "diagonal", "Vector2(1, 4)"),
        };
        [Test]
        public void GetProperties([ValueSource(nameof(GetPropertiesArgsList))] (string lhs, string prop, string rhs) args, [Values] EvaluationMode mode)
        {
            // Tests which construct the matrix and get the named property

            string getter = "_(v1:Vector2, v2:Vector2) = Matrix2x2(v1, v2)." + args.prop;
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(getter, args.lhs, mode),
                              new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string constructorArgs, string result)[] DeterminantArgsList =
        {
            ("(Vector2(1, 0), Vector2(0, 1))", "1"),
            ("(Vector2(1, 2), Vector2(3, 4))", "-2"),
            ("(Vector2(5, 1), Vector2(2, 5))", "23"),
            ("(Vector2(1, 1), Vector2(2, 1))", "-1")
        };
        [Test]
        public void Determinant([ValueSource(nameof(DeterminantArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string getDeterminant = "_(v1:Vector2, v2:Vector2) = Matrix2x2(v1, v2).determinant";
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(getDeterminant, args.lhs, mode),
                              new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string constructorArgs, string resultConstructorArgs)[] TransposeArgsList =
        {
            ("(Vector2(1, 2), Vector2(3, 4))", "(Vector2(1, 3), Vector2(2, 4))"),
        };
        [Test]
        public void Transpose([ValueSource(nameof(TransposeArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string getTranspose = "_(v1:Vector2, v2:Vector2) = Matrix2x2(v1, v2).transpose";
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(getTranspose, args.lhs, mode),
                              new FunctionEvaluation("Matrix2x2", args.rhs, mode));
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
            string matrixMultiply = "_(m:Matrix2x2, v:Vector2) = m.vectorMul(v)";
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
                "Matrix2x2(Vector2(1, 2), Vector2(3, 4))",
                "Matrix2x2.identity",
                "Matrix2x2(Vector2(1, 2), Vector2(3, 4))"
            ),
            (
                "Matrix2x2(Vector2(1, 2), Vector2(3, 4))",
                "Matrix2x2(Vector2(-1, 4), Vector2(-5, 3))",
                "Matrix2x2(Vector2(-11, 10), Vector2(-23, 24))"
            ),
        };
        [Test]
        public void MultiplyMatrix([ValueSource(nameof(MatrixMulArgsList))] (string m1, string m2, string output) args,
            [Values] EvaluationMode mode)
        {
            string matrixMultiply = "_(m1:Matrix2x2, m2:Matrix2x2) = m1.mul(m2)";
            string matrixMultiplyArgs = "(" + args.m1 + ", " + args.m2 + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(matrixMultiply, matrixMultiplyArgs, mode),
                              new ExpressionEvaluation(args.output, mode));
        }
    }
}