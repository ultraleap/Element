using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Matrix4x4 : StandardLibraryFixture
    {
        
        
        public static (string constructorArgs, string constant)[] ConstantArgList =
        {
            ("(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1))", "Matrix4x4.identity"),
        };
        [Test]
        public void Constants([ValueSource(nameof(ConstantArgList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation("Matrix4x4", args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string constructorArgs, string factory, string factoryArgs)[] FactoryArgsList =
        {
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))",
                "fromRows",
                "(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"),
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))",
                "fromCols",
                "(Vector4(1, 5, 9, 13), Vector4(2, 6, 10, 14), Vector4(3, 7, 11, 15), Vector4(4, 8, 12, 16))"),
            ("(Vector4(1, 0, 0, 0), Vector4(0, 2, 0, 0), Vector4(0, 0, 3, 0), Vector4(0, 0, 0, 4))",
                "fromDiagonal",
                "(Vector4(1, 2, 3, 4))"),
        };
        [Test]
        public void Factory([ValueSource(nameof(FactoryArgsList))] (string lhs, string factory, string rhs) args, [Values] EvaluationMode mode)
        {
            string defaultConstructor = "Matrix4x4";
            string altConstructor = "Matrix4x4." + args.factory;
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(defaultConstructor, args.lhs, mode),
                new FunctionEvaluation(altConstructor, args.rhs, mode));
        }
        
        public static (string constructorArgs, string property, string result)[] GetPropertiesArgsList =
        {
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"
                , "xCol", "Vector4(1, 5, 9, 13)"),
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"
                , "yCol", "Vector4(2, 6, 10, 14)"),
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"
                , "zCol", "Vector4(3, 7, 11, 15)"),
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"
                , "wCol", "Vector4(4, 8, 12, 16)"),
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"
                , "xRow", "Vector4(1, 2, 3, 4)"),
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"
                , "yRow", "Vector4(5, 6, 7, 8)"),
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"
                , "zRow", "Vector4(9, 10, 11, 12)"),
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"
                , "wRow", "Vector4(13, 14, 15, 16)"),
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"
                , "diagonal", "Vector4(1, 6, 11, 16)"),
        };
        [Test]
        public void GetProperties([ValueSource(nameof(GetPropertiesArgsList))] (string lhs, string prop, string rhs) args, [Values] EvaluationMode mode)
        {
            // Tests which construct the matrix and get the named property

            string getter = "_(x:Vector4, y:Vector4, z:Vector4, w:Vector4) = Matrix4x4(x, y, z, w)." + args.prop;
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string constructorArgs, string transposedConstructorArgs)[] TransposeArgsList =
        {
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))",
                "(Vector4(1, 5, 9, 13), Vector4(2, 6, 10, 14), Vector4(3, 7, 11, 15), Vector4(4, 8, 12, 16))"),
        };
        [Test]
        public void Transpose([ValueSource(nameof(TransposeArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            // Tests which construct the matrix and get the named property

            string getter = "_(x:Vector4, y:Vector4, z:Vector4, w:Vector4) = Matrix4x4(x, y, z, w).transpose";
            string defaultConstructor = "Matrix4x4";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.lhs, mode),
                new FunctionEvaluation(defaultConstructor, args.rhs, mode));
        }

        public static (string constructorArgs, string result)[] DeterminantArgsList =
        {
            ("(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1))", "1"),
            ("(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))", "0"),
            ("(Vector4(4, 4, 32, 41), Vector4(1, 6, 7, -10), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))", "2320"),
        };
        [Test]
        public void Determinant([ValueSource(nameof(DeterminantArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string getDeterminant = "_(v1, v2, v3, v4) = Matrix4x4(v1, v2, v3, v4).determinant";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getDeterminant, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        
        public static (string matrix, string vector, string output)[] VectorMulArgsList =
        {
            ("Matrix4x4.identity",
                "Vector4(1, 2, 3, 4)",
                "Vector4(1, 2, 3, 4)"),
            ("Matrix4x4.identity",
                "Vector4(1, 1, 1, 0)",
                "Vector4(1, 1, 1, 0)"),
            ("Matrix4x4(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))",
                "Vector4(1, 2, 3, 4)",
                "Vector4(30, 70, 110, 150)"),
        };
        [Test]
        public void MultiplyVector([ValueSource(nameof(VectorMulArgsList))] (string matrix, string vector, string output) args,
            [Values] EvaluationMode mode)
        {
            string matrixMultiply = "_(m:Matrix4x4, v:Vector4) = m.vectorMul(v)";
            string matrixMultiplyArgs = "(" + args.matrix + ", " + args.vector + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(matrixMultiply, matrixMultiplyArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
        
        
        public static (string m1, string m2, string output)[] MatrixMulArgsList =
        {
            (
                "Matrix4x4.identity",
                "Matrix4x4.identity",
                "Matrix4x4.identity"
            ),
            (
                "Matrix4x4(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))",
                "Matrix4x4.identity",
                "Matrix4x4(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))"
            ),
            (
                "Matrix4x4(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))",
                "Matrix4x4.fromCols(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12), Vector4(13, 14, 15, 16))",
                "Matrix4x4(Vector4(30, 70, 110, 150), Vector4(70, 174, 278, 382), Vector4(110, 278, 446, 614), Vector4(150, 382, 614, 846))"
            ),
        };
        [Test]
        public void MultiplyMatrix([ValueSource(nameof(MatrixMulArgsList))] (string m1, string m2, string output) args,
            [Values] EvaluationMode mode)
        {
            string matrixMultiply = "_(m1:Matrix4x4, m2:Matrix4x4) = m1.mul(m2)";
            string matrixMultiplyArgs = "(" + args.m1 + ", " + args.m2 + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(matrixMultiply, matrixMultiplyArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
    }
}
