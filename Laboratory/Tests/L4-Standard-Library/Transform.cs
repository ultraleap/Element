using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Transform : StandardLibraryFixture
    {
        
        public static (string constructorArgs, string factory, string factoryArgs)[] FactoryArgsList =
        {
            (
                "(Matrix4x4.fromRows(Vector4(1, 2, 3, 10), Vector4(4, 5, 6, 11), Vector4(7, 8, 9, 12), Vector4(0, 0, 0, 1)))",
                "fromRotationAndTranslation",
                "(Matrix3x3.fromRows(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9)), Vector3(10, 11, 12))"
            ),
            (
                "(Matrix4x4.fromRows(Vector4(1, 2, 3, 0), Vector4(4, 5, 6, 0), Vector4(7, 8, 9, 0), Vector4(0, 0, 0, 1)))",
                "fromRotation",
                "(Matrix3x3.fromRows(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9)))"
            ),
            (
                "(Matrix4x4.fromRows(Vector4(1, 0, 0, 10), Vector4(0, 1, 0, 11), Vector4(0, 0, 1, 12), Vector4(0, 0, 0, 1)))",
                "fromTranslation",
                "(Vector3(10, 11, 12))"
            ),
        };
        [Test]
        public void Factory([ValueSource(nameof(FactoryArgsList))] (string lhs, string factory, string rhs) args, [Values] EvaluationMode mode)
        {
            string defaultConstructor = "Transform";
            string altConstructor = "Transform." + args.factory;
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(defaultConstructor, args.lhs, mode),
                new FunctionEvaluation(altConstructor, args.rhs, mode));
        }

        public static (string constructorArgs, string property, string result)[] GetPropertiesArgsList =
        {
            (
                "(Matrix4x4.fromRows(Vector4(1, 2, 3, 10), Vector4(4, 5, 6, 11), Vector4(7, 8, 9, 12), Vector4(0, 0, 0, 1)))",
                "translation",
                "Vector3(10, 11, 12)"
            ),
            (
                "(Matrix4x4.fromRows(Vector4(1, 2, 3, 10), Vector4(4, 5, 6, 11), Vector4(7, 8, 9, 12), Vector4(0, 0, 0, 1)))",
                "rotation",
                "Matrix3x3.fromRows(Vector3(1, 2, 3), Vector3(4, 5, 6), Vector3(7, 8, 9))"
            ),
        };
        [Test]
        public void GetProperties([ValueSource(nameof(GetPropertiesArgsList))] (string lhs, string prop, string rhs) args, [Values] EvaluationMode mode)
        {
            // Tests which construct the matrix and get the named property

            string getter = "_(m:Matrix4x4) = Transform(m)." + args.prop;
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        
        public static (string transform, string applyTo, string vector, string output)[] VectorApplyArgsList =
        {
            (
                "Transform.fromTranslation(Vector3.one)",
                "applyToDirection",
                "(Vector3(1, 2, 3))",
                "Vector3(1, 2, 3)"
            ),
            (
                "Transform.fromTranslation(Vector3.one)",
                "applyToPosition",
                "(Vector3(1, 2, 3))",
                "Vector3(2, 3, 4)"
            ),
            (
                "Transform.fromRotationAndTranslation(Matrix3x3(Vector3(0, 1, 0), Vector3(-1, 0, 0), Vector3(0, 0, 1)), Vector3.one)",
                "applyToDirection",
                "(Vector3(1, 2, 3))",
                "Vector3(2, -1, 3)"
            ),
            (
                "Transform.fromRotationAndTranslation(Matrix3x3(Vector3(0, 1, 0), Vector3(-1, 0, 0), Vector3(0, 0, 1)), Vector3.one)",
                "applyToPosition",
                "(Vector3(1, 2, 3))",
                "Vector3(3, 0, 4)"
            ),
        };
        [Test]
        public void VectorApply([ValueSource(nameof(VectorApplyArgsList))]
            (string transform, string application, string vector, string output) args,
            [Values] EvaluationMode mode)
        {
            string applyTransform = "_(v:Vector3) = " + args.transform + "." + args.application + "(v)";
            
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(applyTransform, args.vector, mode),
                new ExpressionEvaluation(args.output, mode));
        }
        
        public static (string axisAngleArgs, string vector, string output)[] FromAxisAngleArgList =
        {
            (
                "(Vector3(0, 0, 1), Num.pi.div(2))",
                "Vector3(1, 1, 0)",
                "Vector3(-1, 1, 0)"
            ),
        };
        [Test]
        public void FromAxisAngle([ValueSource(nameof(FromAxisAngleArgList))]
            (string axisAngleArgs, string vector, string output) args,
            [Values] EvaluationMode mode)
        {
            string applyTransform = "_(v:Vector3, n:Num) = Transform.fromAxisAngle(v, n).applyToDirection(" + args.vector + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(applyTransform, args.axisAngleArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
    }
}
