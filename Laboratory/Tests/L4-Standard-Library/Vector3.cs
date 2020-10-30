using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Vector3 : StandardLibraryFixture
    {
        public static (string constant, string constructorArgs)[] ConstantArgList =
        {
            ("Vector3.zero", "(0, 0, 0)"),
            ("Vector3.one", "(1, 1, 1)"),
            ("Vector3.up", "(0, 0, 1)"),
            ("Vector3.down", "(0, 0, -1)"),
            ("Vector3.right", "(1, 0, 0)"),
            ("Vector3.left", "(-1, 0, 0)"),
            ("Vector3.forward", "(0, 1, 0)"),
            ("Vector3.back", "(0, -1, 0)"),
        };

        [Test]
        public void Constants([ValueSource(nameof(ConstantArgList))] (string lhs, string rhs) args,
            [Values] EvaluationMode mode)
        {
            string defaultConstructor = "Vector3";
            AssertApproxEqual(ValidatedCompilerInput,
                new ExpressionEvaluation(args.lhs, mode),
                new FunctionEvaluation(defaultConstructor, args.rhs, mode));
        }

        public static (string constructorArgs, string property, string result)[] PropertyArgList =
        {
            ("(0, 0, 0)", "magnitudeSquared", "0"),
            ("(1, 0, 0)", "magnitudeSquared", "1"),
            ("(2, 3, 6)", "magnitudeSquared", "49"),
            ("(0, 0, 0)", "magnitude", "0"),
            ("(1, 0, 0)", "magnitude", "1"),
            ("(1, 2, 2)", "magnitude", "3")
        };
        [Test]
        public void Properties([ValueSource(nameof(PropertyArgList))] (string lhs, string property, string rhs) args, [Values] EvaluationMode mode)
        {
            // Tests for scalar properties of vectors
            string getter = "_(x:Num, y:Num, z:Num) = Vector3(x, y, z)." + args.property;

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string constructArgs, string property, string resultArgs)[] VectorPropertyArgList =
        {
            ("(0, 0, 0)", "negate", "(0, 0, 0)"),
            ("(1, 2, 3)", "negate", "(-1, -2, -3)"),
            ("(-1, 2, -3)", "negate", "(1, -2, 3)"),
            ("(1, -2, 3)", "negate", "(-1, 2, -3)"),
            ("(-1, -2, -3)", "negate", "(1, 2, 3)"),
            ("(0, 0, 0)", "normalise", "(Num.NaN, Num.NaN, Num.NaN)"),
            ("(1, 1, 1)", "normalise", "(1.div(3.sqrt), 1.div(3.sqrt), 1.div(3.sqrt))"),
            ("(1, 2, 2)", "normalise", "(1.div(3), 2.div(3), 2.div(3))"),
        };
        [Test]
        public void VectorProperties([ValueSource(nameof(VectorPropertyArgList))] (string lhs, string property, string rhs) args, [Values] EvaluationMode mode)
        {
            string getter = "_(x:Num, y:Num, z:Num) = Vector3(x, y, z)." + args.property;
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.lhs, mode),
                new FunctionEvaluation("Vector3", args.rhs, mode));
        }
        
        public static (string inVector, string scale, string outVector)[] ScaleArgsList =
        {
            ("Vector3(2, 4, 8)", "0", "Vector3(0, 0, 0)"),
            ("Vector3(2, 4, 8)", "2", "Vector3(4, 8, 16)"),
            ("Vector3(2, 4, 8)", "0.5", "Vector3(1, 2, 4)"),
            ("Vector3(2, 4, 8)", "-2", "Vector3(-4, -8, -16)"),
            ("Vector3(2, 4, 8)", "-0.5", "Vector3(-1, -2, -4)"),
            ("Vector3(-2, -4, -8)", "2", "Vector3(-4, -8, -16)"),
            ("Vector3(-2, -4, -8)", "0.5", "Vector3(-1, -2, -4)"),
            ("Vector3(-2, -4, -8)", "-2", "Vector3(4, 8, 16)"),
            ("Vector3(-2, -4, -8)", "-0.5", "Vector3(1, 2, 4)"),
            ("Vector3(2, 4, 8)", "1.div(0)", "Vector3(Num.PositiveInfinity, Num.PositiveInfinity, Num.PositiveInfinity)"),
            ("Vector3(2, 4, 8)", "1.div(-0)", "Vector3(Num.NegativeInfinity, Num.NegativeInfinity, Num.NegativeInfinity)"),
            ("Vector3(2, 4, 8)", "1.div(2)", "Vector3(1, 2, 4)"),
            ("Vector3(2, 4, 8)", "1.div(0.5)", "Vector3(4, 8, 16)"),
            ("Vector3(2, 4, 8)", "1.div(-2)", "Vector3(-1, -2, -4)"),
            ("Vector3(2, 4, 8)", "1.div(-0.5)", "Vector3(-4, -8, -16)"),
            ("Vector3(-2, -4, -8)", "1.div(2)", "Vector3(-1, -2, -4)"),
            ("Vector3(-2, -4, -8)", "1.div(0.5)", "Vector3(-4, -8, -16)"),
            ("Vector3(-2, -4, -8)", "1.div(-2)", "Vector3(1, 2, 4)"),
            ("Vector3(-2, -4, -8)", "1.div(-0.5)", "Vector3(4, 8, 16)"),
        };
        [Test]
        public void Scale([ValueSource(nameof(ScaleArgsList))] (string inVec, string scale, string outVec) args, [Values] EvaluationMode mode)
        {
            string getScaled = "_(v:Vector3, s:Num):Vector3 = v.scale(s)";
            string getScaledArgs = "(" + args.inVec + ", " + args.scale + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getScaled, getScaledArgs, mode),
                new ExpressionEvaluation(args.outVec, mode));
        }
        
        
        public static (string v1, string v2, string operation, string output)[] AddAndSubtractArgsList =
        {
            ("Vector3(0, 0, 0)", "Vector3(0, 0, 0)", "add", "Vector3(0, 0, 0)"),
            ("Vector3(1, 2, 3)", "Vector3(0, 0, 0)", "add",  "Vector3(1, 2, 3)"),
            ("Vector3(1, 2, 3)", "Vector3(2, 1, 3)", "add", "Vector3(3, 3, 6)"),
            ("Vector3(1, 2, 3)", "Vector3(-2, -1, -3)", "add", "Vector3(-1, 1, 0)"),
            ("Vector3(0, 0, 0)", "Vector3(0, 0, 0)", "sub", "Vector3(0, 0, 0)"),
            ("Vector3(1, 2, 3)", "Vector3(0, 0, 0)", "sub", "Vector3(1, 2, 3)"),
            ("Vector3(1, 2, 3)", "Vector3(2, 1, 3)", "sub", "Vector3(-1, 1, 0)"),
            ("Vector3(1, 2, 3)", "Vector3(-2, -1, -3)", "sub", "Vector3(3, 3, 6)"),
        };
        [Test]
        public void AddAndSubtract([ValueSource(nameof(AddAndSubtractArgsList))]
            (string a, string b, string operation, string output) args, [Values] EvaluationMode mode)
        {
            string vectorOperation = "_(a:Vector3, b:Vector3):Vector3 = Vector3." + args.operation + "(a, b)";
            string operationArgs = "(" + args.a + ", " + args.b + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }

        public static (string v1, string v2, string operation, string output)[] DotAndDistanceArgsList =
        {
            ("Vector3(0, 0, 0)", "Vector3(0, 0, 0)", "dot", "0"),
            ("Vector3(1, 1, 1)", "Vector3(0, 0, 0)", "dot", "0"),
            ("Vector3(1, 3, 7)", "Vector3(2, 5, 4)", "dot", "45"),
            ("Vector3(1, 3, 7)", "Vector3(-2, -5, -4)", "dot", "-45"),
            ("Vector3(0, 0, 0)", "Vector3(0, 0, 0)", "distance", "0"),
            ("Vector3(0, 0, 0)", "Vector3(2, 3, 6)", "distance", "7"),
        };
        [Test]
        public void DotAndDistance([ValueSource(nameof(DotAndDistanceArgsList))]
            (string a, string b, string operation, string output) args, [Values] EvaluationMode mode)
        {
            string vectorOperation = "_(a:Vector3, b:Vector3):Num = Vector3." + args.operation + "(a, b)";
            string operationArgs = "(" + args.a + ", " + args.b + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
        
        public static (string v1, string v2, string output)[] AngleArgsList =
        {
            ("Vector3(0, 0, 0)", "Vector3(0, 0, 0)", "Num.NaN"),
            ("Vector3(0, 1, 1)", "Vector3(0, 1, 1)", "0"),
            ("Vector3(0, 1, 1)", "Vector3(1, 1, 0)", "60"),
            ("Vector3(0, 1, 1)", "Vector3(-1, 0, 1)", "60"),
            ("Vector3(0, 1, 1)", "Vector3(1, -1, 1)", "90"),
            ("Vector3(0, 1, 1)", "Vector3(-1, -1, 0)", "120"),
        };
        [Test]
        public void Angle([ValueSource(nameof(AngleArgsList))]
            (string a, string b, string output) args, [Values] EvaluationMode mode)
        {
            string vectorOperation = "_(a:Vector3, b:Vector3):Num = a.angle(b)";
            string operationArgs = "(" + args.a + ", " + args.b + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }

        public static (string v1, string v2, string axis, string output)[] SignedAngleArgsList =
        {
            ("Vector3(0, 0, 0)", "Vector3(0, 0, 0)", "Vector3(0, 0, 1)", "Num.NaN"),
            ("Vector3(1, 0, 0)", "Vector3(0, 1, 0)", "Vector3(0, 0, 1)", "-90"),
            ("Vector3(1, 0, 0)", "Vector3(0, 1, 0)", "Vector3(0, 0, -1)", "90"),
            ("Vector3(0, 1, 0)", "Vector3(0, 0, 1)", "Vector3(1, 0, 0)", "-90"),
            ("Vector3(0, 1, 0)", "Vector3(0, 0, 1)", "Vector3(-1, 0, 0)", "90"),
            ("Vector3(0, 0, 1)", "Vector3(1, 0, 0)", "Vector3(0, 1, 0)", "-90"),
            ("Vector3(0, 0, 1)", "Vector3(1, 0, 0)", "Vector3(0, -1, 0)", "90"),
        };

        [Test]
        public void SignedAngle([ValueSource(nameof(SignedAngleArgsList))]
            (string v1, string v2, string axis, string output) args, [Values] EvaluationMode mode)
        {
            string vectorOperation = "_(a:Vector3, b:Vector3, c:Vector3):Num = a.signedAngle(b, c)";
            string operationArgs = "(" + args.v1 + ", " + args.v2 + "," + args.axis + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }

        public static (string v1, string v2, string output)[] ReflectArgsList =
        {
            ("Vector3(1, 1, 1)", "Vector3(0, 1, 0)", "Vector3(1, -1, 1)"),
            ("Vector3(-1, 1, 1)", "Vector3(0, 1, 0)", "Vector3(-1, -1, 1)"),
            ("Vector3(1, -1, 1)", "Vector3(0, 1, 0)", "Vector3(1, 1, 1)"),
            ("Vector3(-1, -1, 1)", "Vector3(0, 1, 0)", "Vector3(-1, 1, 1)"),
            ("Vector3(1, 1, -1)", "Vector3(1, 0, 0)", "Vector3(-1, 1, -1)"),
            ("Vector3(-1, 1, -1)", "Vector3(1, 0, 0)", "Vector3(1, 1, -1)"),
            ("Vector3(1, -1, -1)", "Vector3(1, 0, 0)", "Vector3(-1, -1, -1)"),
            ("Vector3(-1, -1, -1)", "Vector3(1, 0, 0)", "Vector3(1, -1, -1)"),
        };
        [Test]
        public void Reflect([ValueSource(nameof(ReflectArgsList))]
            (string a, string b, string output) args, [Values] EvaluationMode mode)
        {
            string vectorOperation = "_(a:Vector3, b:Vector3):Vector3 = a.reflect(b)";
            string operationArgs = "(" + args.a + ", " + args.b + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
        
        public static (string v1, string v2, string output)[] CrossArgsList =
        {
            ("Vector3(0, 0, 0)", "Vector3(0, 0, 0)", "Vector3(0, 0, 0)"),
            ("Vector3(1, 1, 1)", "Vector3(1, 1, 1)", "Vector3(0, 0, 0)"),
            ("Vector3(1, 1, 1)", "Vector3(1, 1, 0)", "Vector3(-1, 1, 0)"),
            ("Vector3(3, 4, 5)", "Vector3(1, 0, 0)", "Vector3(0, 5, -4)"),
        };
        [Test]
        public void CrossProduct([ValueSource(nameof(CrossArgsList))]
            (string a, string b, string output) args, [Values] EvaluationMode mode)
        {
            string vectorOperation = "_(a:Vector3, b:Vector3):Vector3 = a.cross(b)";
            string operationArgs = "(" + args.a + ", " + args.b + ")";

            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorOperation, operationArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }
        
        
        public static (string lerpArgs, string result)[] LerpArgsList =
        {
            ("(-0.25, Vector3.zero, Vector3.one)", "Vector3.one.scale(-0.25)"),    //extrapolation
            ("(0, Vector3.zero, Vector3.one)", "Vector3.one.scale(0)"),
            ("(0.25, Vector3.zero, Vector3.one)", "Vector3.one.scale(0.25)"),
            ("(0.5, Vector3.zero, Vector3.one)", "Vector3.one.scale(0.5)"),
            ("(0.75, Vector3.zero, Vector3.one)", "Vector3.one.scale(0.75)"),
            ("(1, Vector3.zero, Vector3.one)", "Vector3.one"),
            ("(1.25, Vector3.zero, Vector3.one)", "Vector3.one.scale(1.25)"),     //extrapolation
        };
        [Test]
        public void Lerp([ValueSource(nameof(LerpArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string vectorLerp = "Vector3.lerp";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorLerp, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
    }
}