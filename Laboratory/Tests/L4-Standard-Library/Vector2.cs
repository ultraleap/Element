using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Vector2 : StandardLibraryFixture
    {

        public static (string constant, string constructorArgs)[] ConstantArgList =
        {
            ("Vector2.zero", "(0, 0)"),
            ("Vector2.one", "(1, 1)"),
            ("Vector2.right", "(1, 0)"),
            ("Vector2.left", "(-1, 0)"),
            ("Vector2.up", "(0, 1)"),
            ("Vector2.down", "(0, -1)"),

        };

        [Test]
        public void Constants([ValueSource(nameof(ConstantArgList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                              new ExpressionEvaluation(args.lhs, mode),
                              new FunctionEvaluation("Vector2", args.rhs, mode));
        }

        public static (string constructorArgs, string magnitudeSquared)[] MagnitudeSquaredArgList =
        {
            ("(0, 0)", "0"),
            ("(1, 0)", "1"),
            ("(3, 4)", "25"),
        };
        [Test]
        public void MagnitudeSquared([ValueSource(nameof(MagnitudeSquaredArgList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string magnitudeSquaredGetter = "_(x:Num, y:Num) = Vector2(x, y).magnitudeSquared";
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(magnitudeSquaredGetter, args.lhs, mode),
                              new ExpressionEvaluation(args.rhs, mode));
        }


        public static (string constructorArgs, string magnitude)[] MagnitudeArgsList =
        {
            ("(0, 0)", "0"),
            ("(1, 0)", "1"),
            ("(3, 4)", "5")
        };
        [Test]
        public void Magntitude([ValueSource(nameof(MagnitudeArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string magnitudeGetter = "_(x:Num, y:Num) = Vector2(x, y).magnitude";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(magnitudeGetter, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string negatorArgs, string negatedArgs)[] NegateArgsList =
        {
            ("(0, 0)", "(0, 0)"),
            ("(1, 2)", "(-1, -2)"),
            ("(-1, 2)", "(1, -2)"),
            ("(1, -2)", "(-1, 2)"),
            ("(-1, -2)", "(1, 2)")
        };
        [Test]
        public void Negate([ValueSource(nameof(NegateArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string getNegated = "_(x:Num, y:Num) = Vector2(x, y).negate";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getNegated, args.lhs, mode),
                new FunctionEvaluation("Vector2", args.rhs, mode));
        }

        public static (string constructorArgs, string normalisedArgs)[] NormaliseArgsList =
        {
            ("(0, 0)", "(Num.NaN, Num.NaN)"),
            ("(1, 1)", "(1.div(2.sqrt), 1.div(2.sqrt))"),
            ("(3, 4)", "(3.div(5), 4.div(5))")
        };
        [Test]
        public void Normalise([ValueSource(nameof(NormaliseArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string getNormalised = "_(x:Num, y:Num) = Vector2(x, y).normalise";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getNormalised, args.lhs, mode),
                new FunctionEvaluation("Vector2", args.rhs, mode));
        }

        public static (string scaleArgs, string result)[] ScaleArgsList =
        {
            ("(Vector2(2, 4), 0)", "Vector2(0, 0)"),
            ("(Vector2(2, 4), 2)", "Vector2(4, 8)"),
            ("(Vector2(2, 4), 0.5)", "Vector2(1, 2)"),
            ("(Vector2(2, 4), -2)", "Vector2(-4, -8)"),
            ("(Vector2(2, 4), -0.5)", "Vector2(-1, -2)"),
            ("(Vector2(-2, -4), 2)", "Vector2(-4, -8)"),
            ("(Vector2(-2, -4), 0.5)", "Vector2(-1, -2)"),
            ("(Vector2(-2, -4), -2)", "Vector2(4, 8)"),
            ("(Vector2(-2, -4), -0.5)", "Vector2(1, 2)"),
            ("(Vector2(2, 4), 1.div(0))", "Vector2(Num.PositiveInfinity, Num.PositiveInfinity)"),
            ("(Vector2(2, 4), 1.div(-0))", "Vector2(Num.NegativeInfinity, Num.NegativeInfinity)"),
            ("(Vector2(2, 4), 1.div(2))", "Vector2(1, 2)"),
            ("(Vector2(2, 4), 1.div(0.5))", "Vector2(4, 8)"),
            ("(Vector2(2, 4), 1.div(-2))", "Vector2(-1, -2)"),
            ("(Vector2(2, 4), 1.div(-0.5))", "Vector2(-4, -8)"),
            ("(Vector2(-2, -4), 1.div(2))", "Vector2(-1, -2)"),
            ("(Vector2(-2, -4), 1.div(0.5))", "Vector2(-4, -8)"),
            ("(Vector2(-2, -4), 1.div(-2))", "Vector2(1, 2)"),
            ("(Vector2(-2, -4), 1.div(-0.5))", "Vector2(4, 8)"),
        };
        [Test]
        public void Scale([ValueSource(nameof(ScaleArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string getScaled = "_(v:Vector2, s:Num) = v.scale(s)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getScaled, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }


        public static (string addArgs, string result)[] AddArgsList =
        {
            ("(Vector2(0, 0), Vector2(0, 0))", "Vector2(0, 0)"),
            ("(Vector2(1, 2), Vector2(0, 0))", "Vector2(1, 2)"),
            ("(Vector2(1, 2), Vector2(2, 1))", "Vector2(3, 3)"),
            ("(Vector2(1, 2), Vector2(-2, -1))", "Vector2(-1, 1)"),

        };
        [Test]
        public void VectorAdd([ValueSource(nameof(AddArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string vectorAdd = "_(a:Vector2, b:Vector2) = a.add(b)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorAdd, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string subArgs, string result)[] SubArgsList =
        {
            ("(Vector2(0, 0), Vector2(0, 0))", "Vector2(0, 0)"),
            ("(Vector2(1, 2), Vector2(0, 0))", "Vector2(1, 2)"),
            ("(Vector2(1, 2), Vector2(2, 1))", "Vector2(-1, 1)"),
            ("(Vector2(1, 2), Vector2(-2, -1))", "Vector2(3, 3)"),

        };
        [Test]
        public void VectorSubtract([ValueSource(nameof(SubArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string vectorAdd = "_(a:Vector2, b:Vector2) = a.sub(b)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorAdd, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }


        public static (string dotArgs, string result)[] DotArgsList =
        {
            ("(Vector2(0, 0), Vector2(0, 0))", "0"),
            ("(Vector2(1, 1), Vector2(0, 0))", "0"),
            ("(Vector2(1, 3), Vector2(2, 5))", "17"),
            ("(Vector2(1, 3), Vector2(-2, -5))", "-17"),

        };
        [Test]
        public void DotProduct([ValueSource(nameof(DotArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string vectorDot = "_(a:Vector2, b:Vector2) = a.dot(b)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorDot, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string distanceArgs, string result)[] DistanceArgsList =
        {
            ("(Vector2(0, 0), Vector2(0, 0))", "0"),
            ("(Vector2(0, 0), Vector2(3, 4))", "5"),
        };
        [Test]
        public void Distance([ValueSource(nameof(DistanceArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string vectorDistance = "_(a:Vector2, b:Vector2) = a.distance(b)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorDistance, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }


        public static (string angleArgs, string result)[] AngleArgsList =
        {
            ("(Vector2(0, 0), Vector2(0, 0))", "Num.NaN"),
            ("(Vector2(0, 1), Vector2(0, 1))", "0"),
            ("(Vector2(0, 1), Vector2(1, 1))", "45"),
            ("(Vector2(0, 1), Vector2(-1, 1))", "45"),
            ("(Vector2(0, 1), Vector2(1, -1))", "135"),
            ("(Vector2(0, 1), Vector2(-1, -1))", "135"),

        };
        [Test]
        public void Angle([ValueSource(nameof(AngleArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string vectorAngle = "_(a:Vector2, b:Vector2) = a.angle(b)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorAngle, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string angleArgs, string result)[] SignedAngleArgsList =
        {
            ("(Vector2(0, 0), Vector2(0, 0))", "Num.NaN"),
            ("(Vector2(0, 1), Vector2(0, 1))", "0"),
            ("(Vector2(0, 1), Vector2(1, 1))", "45"),
            ("(Vector2(0, 1), Vector2(-1, 1))", "-45"),
            ("(Vector2(0, 1), Vector2(1, -1))", "135"),
            ("(Vector2(0, 1), Vector2(-1, -1))", "-135"),

        };
        [Test]
        public void SignedAngle([ValueSource(nameof(SignedAngleArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string vectorSignedAngle = "_(a:Vector2, b:Vector2) = a.signedAngle(b)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorSignedAngle, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string reflectArgs, string result)[] ReflectArgsList =
        {
            ("(Vector2(1, 1), Vector2(0, 1))", "Vector2(1, -1)"),
            ("(Vector2(-1, 1), Vector2(0, 1))", "Vector2(-1, -1)"),
            ("(Vector2(1, -1), Vector2(0, 1))", "Vector2(1, 1)"),
            ("(Vector2(-1, -1), Vector2(0, 1))", "Vector2(-1, 1)"),
            ("(Vector2(1, 1), Vector2(1, 0))", "Vector2(-1, 1)"),
            ("(Vector2(-1, 1), Vector2(1, 0))", "Vector2(1, 1)"),
            ("(Vector2(1, -1), Vector2(1, 0))", "Vector2(-1, -1)"),
            ("(Vector2(-1, -1), Vector2(1, 0))", "Vector2(1, -1)"),
        };
        [Test]
        public void Reflect([ValueSource(nameof(ReflectArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string vectorReflect = "_(a:Vector2, b:Vector2) = a.reflect(b)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorReflect, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string lerpArgs, string result)[] LerpArgsList =
        {
            ("(-0.25, Vector2.zero, Vector2.one)", "Vector2(-0.25, -0.25)"),    //extrapolation
            ("(0, Vector2.zero, Vector2.one)", "Vector2(0, 0)"),
            ("(0.25, Vector2.zero, Vector2.one)", "Vector2(0.25, 0.25)"),
            ("(5, Vector2.zero, Vector2.one)", "Vector2(0.5, 0.5)"),
            ("(0.75, Vector2.zero, Vector2.one)", "Vector2(0.75, 0.75)"),
            ("(1, Vector2.zero, Vector2.one)", "Vector2(1, 1)"),
            ("(1.25, Vector2.zero, Vector2.one)", "Vector2(1.25, 1.25)"),     //extrapolation
        };
        [Test]
        public void Lerp([ValueSource(nameof(LerpArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string vectorLerp = "Vector2.lerp";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(vectorLerp, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
    }
}