using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Vector4 : StandardLibraryFixture
    {
        
        public static (string constant, string constructorArgs)[] ConstantArgList =
        {
            ("Vector4.zero", "(0, 0, 0, 0)"),
            ("Vector4.one", "(1, 1, 1, 1)"),
        };
        [Test]
        public void Constants([ValueSource(nameof(ConstantArgList))] (string lhs, string rhs) args,
            [Values] EvaluationMode mode)
        {
            string defaultConstructor = "Vector4";
            AssertApproxEqual(ValidatedCompilerInput,
                new ExpressionEvaluation(args.lhs, mode),
                new FunctionEvaluation(defaultConstructor, args.rhs, mode));
        }
        
        public static (string constructorArgs, string property, string result)[] PropertyArgList =
        {
            ("(0, 0, 0, 0)", "magnitudeSquared", "0"),
            ("(1, 0, 0, 0)", "magnitudeSquared", "1"),
            ("(2, 3, 6, 24)", "magnitudeSquared", "625"),
            ("(0, 0, 0, 0)", "magnitude", "0"),
            ("(1, 0, 0, 0)", "magnitude", "1"),
            ("(2, 3, 6, 24)", "magnitude", "25")
        };

        [Test]
        public void Properties([ValueSource(nameof(PropertyArgList))] (string lhs, string property, string rhs) args,
            [Values] EvaluationMode mode)
        {
            // Tests for scalar properties of vectors
            string getter = "_(x:Num, y:Num, z:Num, w:Num) = Vector4(x, y, z, w)." + args.property;

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string constructArgs, string property, string resultArgs)[] VectorPropertyArgList =
        {
            ("(0, 0, 0, 0)", "negate", "(0, 0, 0, 0)"),
            ("(1, 2, 3, 4)", "negate", "(-1, -2, -3, -4)"),
            ("(-1, 2, -3, -4)", "negate", "(1, -2, 3, 4)"),
            ("(1, -2, 3, -4)", "negate", "(-1, 2, -3, 4)"),
            ("(-1, -2, -3, -4)", "negate", "(1, 2, 3, 4)"),
            ("(0, 0, 0, 0)", "normalise", "(Num.NaN, Num.NaN, Num.NaN, Num.NaN)"),
            ("(1, 1, 1, 1)", "normalise", "(1.div(2), 1.div(2), 1.div(2), 1.div(2))"),
            ("(2, 2, 4, 5)", "normalise", "(2.div(7), 2.div(7), 4.div(7), 5.div(7))"),
        };
        [Test]
        public void VectorProperties([ValueSource(nameof(VectorPropertyArgList))] (string lhs, string property, string rhs) args, [Values] EvaluationMode mode)
        {
            string getter = "_(x:Num, y:Num, z:Num, w:Num) = Vector4(x, y, z, w)." + args.property;
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.lhs, mode),
                new FunctionEvaluation("Vector4", args.rhs, mode));
        }
        
        public static (string inVector, string scale, string outVector)[] ScaleArgsList =
        {
            ("Vector4(2, 4, 8, 16)", "0", "Vector4(0, 0, 0, 0)"),
            ("Vector4(2, 4, 8, 16)", "2", "Vector4(4, 8, 16, 32)"),
            ("Vector4(2, 4, 8, 16)", "0.5", "Vector4(1, 2, 4, 8)"),
            ("Vector4(2, 4, 8, 16)", "-2", "Vector4(-4, -8, -16, -32)"),
            ("Vector4(2, 4, 8, 16)", "-0.5", "Vector4(-1, -2, -4, -8)"),
            ("Vector4(-2, -4, -8, -16)", "2", "Vector4(-4, -8, -16, -32)"),
            ("Vector4(-2, -4, -8, -16)", "0.5", "Vector4(-1, -2, -4, -8)"),
            ("Vector4(-2, -4, -8, -16)", "-2", "Vector4(4, 8, 16, 32)"),
            ("Vector4(-2, -4, -8, -16)", "-0.5", "Vector4(1, 2, 4, 8)"),
            ("Vector4(2, 4, 8, 16)", "1.div(0)", "Vector4(Num.PositiveInfinity, Num.PositiveInfinity, Num.PositiveInfinity, Num.PositiveInfinity)"),
            ("Vector4(2, 4, 8, 16)", "1.div(-0)", "Vector4(Num.NegativeInfinity, Num.NegativeInfinity, Num.NegativeInfinity, Num.NegativeInfinity)"),
            ("Vector4(2, 4, 8, 16)", "1.div(2)", "Vector4(1, 2, 4, 8)"),
            ("Vector4(2, 4, 8, 16)", "1.div(0.5)", "Vector4(4, 8, 16, 32)"),
            ("Vector4(2, 4, 8, 16)", "1.div(-2)", "Vector4(-1, -2, -4, -8)"),
            ("Vector4(2, 4, 8, 16)", "1.div(-0.5)", "Vector4(-4, -8, -16, -32)"),
            ("Vector4(-2, -4, -8, -16)", "1.div(2)", "Vector4(-1, -2, -4, -8)"),
            ("Vector4(-2, -4, -8, -16)", "1.div(0.5)", "Vector4(-4, -8, -16, -32)"),
            ("Vector4(-2, -4, -8, -16)", "1.div(-2)", "Vector4(1, 2, 4, 8)"),
            ("Vector4(-2, -4, -8, -16)", "1.div(-0.5)", "Vector4(4, 8, 16, 32)"),
        };
        [Test]
        public void Scale([ValueSource(nameof(ScaleArgsList))] (string inVec, string scale, string outVec) args, [Values] EvaluationMode mode)
        {
            string getScaled = "_(v:Vector4, s:Num):Vector4 = v.scale(s)";
            string getScaledArgs = "(" + args.inVec + ", " + args.scale + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getScaled, getScaledArgs, mode),
                new ExpressionEvaluation(args.outVec, mode));
        }
        
        public static (string v1, string v2, string operation, string output)[] AddAndSubtractArgsList =
        {
            ("Vector4(0, 0, 0, 0)", "Vector4(0, 0, 0, 0)", "add", "Vector4(0, 0, 0, 0)"),
            ("Vector4(1, 2, 3, 4)", "Vector4(0, 0, 0, 0)", "add",  "Vector4(1, 2, 3, 4)"),
            ("Vector4(1, 2, 3, 4)", "Vector4(2, 1, 3, 6)", "add", "Vector4(3, 3, 6, 10)"),
            ("Vector4(1, 2, 3, 4)", "Vector4(-2, -1, -3, -6)", "add", "Vector4(-1, 1, 0, -2)"),
            ("Vector4(0, 0, 0, 0)", "Vector4(0, 0, 0, 0)", "sub", "Vector4(0, 0, 0, 0)"),
            ("Vector4(1, 2, 3, 4)", "Vector4(0, 0, 0, 0)", "sub", "Vector4(1, 2, 3, 4)"),
            ("Vector4(1, 2, 3, 4)", "Vector4(2, 1, 3, 6)", "sub", "Vector4(-1, 1, 0, -2)"),
            ("Vector4(1, 2, 3, 4)", "Vector4(-2, -1, -3, -6)", "sub", "Vector4(3, 3, 6, 10)"),
        };
        [Test]
        public void AddAndSubtract([ValueSource(nameof(AddAndSubtractArgsList))]
            (string a, string b, string operation, string output) args, [Values] EvaluationMode mode)
        {
            string vectorOperation = "_(a:Vector4, b:Vector4):Vector4 = Vector4." + args.operation + "(a, b)";
            string operationArgs = "(" + args.a + ", " + args.b + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
        
        public static (string v1, string v2, string output)[] DotArgs =
        {
            ("Vector4(0, 0, 0, 0)", "Vector4(0, 0, 0, 0)", "0"),
            ("Vector4(1, 1, 1, 9)", "Vector4(0, 0, 0, 0)",  "0"),
            ("Vector4(1, 3, 7, 9)", "Vector4(2, 5, 4, 5)", "90"),
            ("Vector4(1, 3, 7, 9)", "Vector4(-2, -5, -4, -5)",  "-90"),
        };
        [Test]
        public void DotProduct([ValueSource(nameof(DotArgs))]
            (string a, string b, string output) args, [Values] EvaluationMode mode)
        {
            string vectorOperation = "_(a:Vector4, b:Vector4):Num = a.dot(b)";
            string operationArgs = "(" + args.a + ", " + args.b + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }

        public static (string v1, string v2, string output)[] ReflectArgsList =
        {
            ("Vector4(1, 0, 0, 0)", "Vector4(1, 0, 0, 0)", "Vector4(-1, 0, 0, 0)"),
        };
        [Test]
        public void Reflect([ValueSource(nameof(ReflectArgsList))]
            (string a, string b, string output) args, [Values] EvaluationMode mode)
        {
            string vectorOperation = "_(a:Vector4, b:Vector4):Vector4 = a.reflect(b)";
            string operationArgs = "(" + args.a + ", " + args.b + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
        
        public static (string lerpArgs, string result)[] LerpArgsList =
        {
            ("(-0.25, Vector4.zero, Vector4.one)", "Vector4.one.scale(-0.25)"),    //extrapolation
            ("(0, Vector4.zero, Vector4.one)", "Vector4.one.scale(0)"),
            ("(0.25, Vector4.zero, Vector4.one)", "Vector4.one.scale(0.25)"),
            ("(0.5, Vector4.zero, Vector4.one)", "Vector4.one.scale(0.5)"),
            ("(0.75, Vector4.zero, Vector4.one)", "Vector4.one.scale(0.75)"),
            ("(1, Vector4.zero, Vector4.one)", "Vector4.one"),
            ("(1.25, Vector4.zero, Vector4.one)", "Vector4.one.scale(1.25)"),     //extrapolation
        };
        [Test]
        public void Lerp([ValueSource(nameof(LerpArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string vectorLerp = "Vector4.lerp";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorLerp, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string input, string output)[] ToVector3ArgsList =
        {
            ("Vector4(1, 2, 3, 4)", "Vector3(1, 2, 3)"),
            ("Vector4(1, 2, 3, 0)",  "Vector3(1, 2, 3)"),
        };
        [Test]
        public void ToVector3([ValueSource(nameof(ToVector3ArgsList))]
            (string input, string output) args, [Values] EvaluationMode mode)
        {
            string operationArgs = "(" + args.input + ")";
            string vectorOperation = "Vector4.toVector3";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
        
        public static (string input, string w, string output)[] FromVector3ArgsList =
        {
            ("Vector3(1, 2, 3)", "0", "Vector4(1, 2, 3, 0)"),
            ("Vector3(1, 2, 3)",  "4", "Vector4(1, 2, 3, 4)"),
        };
        [Test]
        public void FromVector3([ValueSource(nameof(FromVector3ArgsList))]
            (string input, string w, string output) args, [Values] EvaluationMode mode)
        {
            string operationArgs = "(" + args.input + "," + args.w + ")";
            string vectorOperation = "Vector4.fromVector3";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
    }
}