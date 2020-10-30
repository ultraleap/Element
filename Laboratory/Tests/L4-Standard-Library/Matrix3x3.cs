using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Matrix3x3 : StandardLibraryFixture
    {

        public static (string constructorArgs, string constant)[] ConstantArgList =
        {
            ("(Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1))", "Matrix3x3.identity"),
        };
        [Test]
        public void Constants([ValueSource(nameof(ConstantArgList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation("Matrix3x3", args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string constructorArgs, string factory, string factoryArgs)[] FactoryArgsList =
        {
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))",
                "fromRows",
                "(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))"),
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))",
                "fromCols",
                "(Vector3(1, 4, 7), Vector3(2, 5, 8), Vector3(3, 6, 9))"),
            ("(Vector3(1, 0, 0), Vector3(0, 2, 0), Vector3(0, 0, 3))",
                "fromDiagonal",
                "(Vector3(1, 2, 3))"),
        };
        [Test]
        public void Factory([ValueSource(nameof(FactoryArgsList))] (string lhs, string factory, string rhs) args, [Values] EvaluationMode mode)
        {
            string defaultConstructor = "Matrix3x3";
            string altConstructor = "Matrix3x3." + args.factory;
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(defaultConstructor, args.lhs, mode),
                new FunctionEvaluation(altConstructor, args.rhs, mode));
        }

        public static (string constructorArgs, string property, string result)[] GetPropertiesArgsList =
        {
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))", "xCol", "Vector3(1, 4, 7)"),
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))", "yCol", "Vector3(2, 5, 8)"),
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))", "zCol", "Vector3(3, 6, 9)"),
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))", "xRow", "Vector3(1, 2, 3)"),
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))", "yRow", "Vector3(4, 5, 6)"),
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))", "zRow", "Vector3(7, 8, 9)"),
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))", "diagonal", "Vector3(1, 5, 9)"),
        };
        [Test]
        public void GetProperties([ValueSource(nameof(GetPropertiesArgsList))] (string lhs, string prop, string rhs) args, [Values] EvaluationMode mode)
        {
            // Tests which construct the matrix and get the named property

            string getter = "_(x:Vector3, y:Vector3, z:Vector3) = Matrix3x3(x, y, z)." + args.prop;
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string constructorArgs, string resultConstructorArgs)[] TransposeArgsList =
        {
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))", "(Vector3(1, 4, 7), Vector3(2, 5, 8), Vector3(3, 6, 9))"),
        };
        [Test]
        public void Transpose([ValueSource(nameof(TransposeArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string getTranspose = "_(v1, v2, v3) = Matrix3x3(v1, v2, v3).transpose";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getTranspose, args.lhs, mode),
                new FunctionEvaluation("Matrix3x3", args.rhs, mode));
        }
        
        public static (string constructorArgs, string result)[] DeterminantArgsList =
        {
            ("(Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1))", "1"),
            ("(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))", "0"),
            ("(Vector3(2, 0, 0), Vector3(0, 2, 0), Vector3(0, 0, 2))", "8"),
        };
        [Test]
        public void Determinant([ValueSource(nameof(DeterminantArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string getDeterminant = "_(v1:Vector3, v2:Vector3, v3:Vector3) = Matrix3x3(v1, v2, v3).determinant";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getDeterminant, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        

        public static (string matrix, string vector, string output)[] VectorMulArgsList =
        {
            ("Matrix3x3.identity",
                "Vector3(1, 2, 3)",
                "Vector3(1, 2, 3)"),
            ("Matrix3x3.fromCols(Vector3(1, 4, 7), Vector3(2, 5, 8), Vector3(3, 6, 9))",
                "Vector3(1, 2, 3)",
                "Vector3(14, 32, 50)"),
        };
        [Test]
        public void MultiplyVector([ValueSource(nameof(VectorMulArgsList))] (string matrix, string vector, string output) args,
            [Values] EvaluationMode mode)
        {
            string matrixMultiply = "_(m:Matrix3x3, v:Vector3) = m.vectorMul(v)";
            string matrixMultiplyArgs = "(" + args.matrix + ", " + args.vector + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(matrixMultiply, matrixMultiplyArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
        
        public static (string m1, string m2, string output)[] MatrixMulArgsList =
        {
            (
                "Matrix3x3.identity",
                "Matrix3x3.identity",
                "Matrix3x3.identity"
            ),
            (
                "Matrix3x3(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))",
                "Matrix3x3.identity",
                "Matrix3x3(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))"
            ),
            (
                "Matrix3x3(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))",
                "Matrix3x3.fromCols(Vector3(9, 6, 3), Vector3(8, 5, 2), Vector3(7, 4, 1))",
                "Matrix3x3.fromCols(Vector3(30, 84, 138), Vector3(24, 69, 114), Vector3(18, 54, 90))"
            ),
        };
        [Test]
        public void MultiplyMatrix([ValueSource(nameof(MatrixMulArgsList))] (string m1, string m2, string output) args,
            [Values] EvaluationMode mode)
        {
            string matrixMultiply = "_(m1:Matrix3x3, m2:Matrix3x3) = m1.mul(m2)";
            string matrixMultiplyArgs = "(" + args.m1 + ", " + args.m2 + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(matrixMultiply, matrixMultiplyArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
    }
}
