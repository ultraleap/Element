using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Paths : StandardLibraryFixture
    {
        public static (string lhs, string rhs)[] ScaleBoundsArgs =
        {
            ("(Bounds(0, 1), Bounds(3, 4), 3)", "0"),
            ("(Bounds(0, 1), Bounds(3, 4), 3.5)", "0.5"),
            ("(Bounds(0, 1), Bounds(3, 4), 4)", "1"),
            ("(Bounds(2, 3), Bounds(3, 6), 3)", "2"),
            ("(Bounds(2, 3), Bounds(3, 6), 4.5)", "2.5"),
            ("(Bounds(2, 3), Bounds(3, 6), 6)", "3"),
        };
        [Test]
        public void ScaleBounds([ValueSource(nameof(ScaleBoundsArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(oldBounds:Bounds, newBounds:Bounds, t:Num):Num = " +
                                  "Path(StandardPaths.line(Vector3.zero, Vector3.one).at, oldBounds).changeDomain(newBounds).at(t).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string rhs)[] PingPongArgs =
        {
            ("0", "0"),
            ("0.2", "0.4"),
            ("0.25", "0.5"),
            ("0.4", "0.8"),
            ("0.5", "1"),
            ("0.6", "0.8"),
            ("0.75", "0.5"),
            ("0.8", "0.4"),
            ("1", "0"),
        };
        [Test]
        public void PingPong([ValueSource(nameof(PingPongArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testArgs = "(" + args.lhs+ ")";
            string testFunction = "_(u:Num):Num = StandardPaths.line(Vector3.zero, Vector3.one).pingPong.at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string rhs)[] PingPong24Args =
        {
            ("2", "2"),
            ("2.4", "2.8"),
            ("2.5", "3"),
            ("2.8", "3.6"),
            ("3", "4"),
            ("3.2", "3.6"),
            ("3.5", "3"),
            ("3.6", "2.8"),
            ("4", "2"),
        };
        [Test]
        public void PingPong24([ValueSource(nameof(PingPong24Args))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testArgs = "(" + args.lhs+ ")";
            string testFunction = "_(u:Num):Num =" +
                                  "Path(StandardPaths.line(Vector3.zero, Vector3.one).at, Bounds(2, 4))" +
                                  ".pingPong.at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string lhs, string rhs)[] OscillateArgs =
        {
            ("0", "0"),
            ("0.25", "0.5"),
            ("0.5", "1"),
            ("0.75", "0.5"),
            ("1", "0"),
        };
        [Test]
        public void Oscillate([ValueSource(nameof(OscillateArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testArgs = "(" + args.lhs+ ")";
            string testFunction = "_(u:Num):Num = StandardPaths.line(Vector3.zero, Vector3.one).oscillate.at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string rhs)[] ReverseArgs =
        {
            ("0", "1"),
            ("0.25", "0.75"),
            ("0.5", "0.5"),
            ("0.75", "0.25"),
            ("1", "0"),
        };
        [Test]
        public void Reverse([ValueSource(nameof(ReverseArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testArgs = "(" + args.lhs+ ")";
            string testFunction = "_(u:Num):Num = StandardPaths.line(Vector3.zero, Vector3.one).reverse.at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string rhs)[] EasingArgs =
        {
            ("0", "0"),
            ("0.25", "5.div(32)"),
            ("0.5", "0.5"),
            ("0.75", "27.div(32)"),
            ("1", "1"),
        };
        [Test]
        public void Easing([ValueSource(nameof(EasingArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testArgs = "(" + args.lhs+ ")";
            string testFunction = "_(u:Num):Num = StandardPaths.line(Vector3.zero, Vector3.one).easing.at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, testArgs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string lhs, string rhs)[] RepeatArgs =
        {
            ("(1, 0)", "0"),
            ("(1, 0.5)", "0.5"),
            ("(1, 0.9)", "0.9"),
            ("(2, 0)", "0"),
            ("(2, 0.25)", "0.5"),
            ("(2, 0.4)", "0.8"),
            ("(2, 0.5)", "0"),
            ("(2, 0.75)", "0.5"),
            ("(2, 0.9)", "0.8"),
        };
        [Test]
        public void PathRepeat([ValueSource(nameof(RepeatArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(n:Num, u:Num):Num = StandardPaths.line(Vector3.zero, Vector3.one).repeat(n).at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        public static (string op, string lhs, string rhs)[] BinaryOpArgs =
        {
            ("Vector3.add", "(0)", "1"),
            ("Vector3.add", "(0.5)", "2"),
            ("Vector3.add", "(1)", "3"),
            ("Vector3.sub", "(0)", "-1"),
            ("Vector3.sub", "(0.5)", "-1"),
            ("Vector3.sub", "(1)", "-1"),
        };
        [Test]
        public void PathBinaryOp([ValueSource(nameof(BinaryOpArgs))] (string op, string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            // Apply the binary operation to the two lines from (0, 0, 0) to (1, 1, 1), and from (1, 1, 1) to (2, 2, 2)
            string testFunction = "_(u:Num):Num = " +
                                  "Path.applyBinaryVectorOperation(" +
                                  "StandardPaths.line(Vector3.zero, Vector3.one)," +
                                  "StandardPaths.line(Vector3.one, Vector3.one.scale(2))," +
                                  args.op + ").at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string rhs)[] PathAddArgs =
        {
            ("(0)", "1"),
            ("(0.5)", "2"),
            ("(1)", "3"),
        };
        [Test]
        public void PathAdd([ValueSource(nameof(PathAddArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            // Add the two lines from (0, 0, 0) to (1, 1, 1), and from (1, 1, 1) to (2, 2, 2)
            string testFunction = "_(u:Num):Num = " +
                                  "Path.add(" +
                                  "StandardPaths.line(Vector3.zero, Vector3.one)," +
                                  "StandardPaths.line(Vector3.one, Vector3.one.scale(2))).at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string op, string lhs, string rhs)[] UnaryOpArgs =
        {
            ("_(v:Vector3):Vector3 = v.add(Vector3.one)", "(0)", "1"),
            ("_(v:Vector3):Vector3 = v.add(Vector3.one)", "(0.5)", "1.5"),
            ("_(v:Vector3):Vector3 = v.add(Vector3.one)", "(0.9)", "1.9"),
            ("_(v:Vector3):Vector3 = v.sub(Vector3.one)", "(0)", "-1"),
            ("_(v:Vector3):Vector3 = v.sub(Vector3.one)", "(0.5)", "-0.5"),
            ("_(v:Vector3):Vector3 = v.sub(Vector3.one)", "(0.9)", "-0.1"),
        };
        [Test]
        public void PathUnaryOp([ValueSource(nameof(UnaryOpArgs))] (string op, string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(u:Num):Num = Path.applyUnaryVectorOperation(" +
                                  "StandardPaths.line(Vector3.zero, Vector3.one)," +
                                  args.op + ").at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string rhs)[] ApplyTransformArgs =
        {
            ("(Transform.fromTranslation(Vector3.one), 0)", "1"),
            ("(Transform.fromTranslation(Vector3.one), 0.5)", "1.5"),
            ("(Transform.fromTranslation(Vector3.one), 0.9)", "1.9"),
        };
        [Test]
        public void ApplyTransform([ValueSource(nameof(ApplyTransformArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(tr:Transform, u:Num):Num = Path.applyTransform(StandardPaths.line(Vector3.zero, Vector3.one), tr).at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string rhs)[] ApplyOffsetArgs =
        {
            ("(Vector3.one, 0)", "1"),
            ("(Vector3.one, 0.5)", "1.5"),
            ("(Vector3.one, 0.9)", "1.9"),
        };
        [Test]
        public void ApplyOffset([ValueSource(nameof(ApplyOffsetArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction =
                "_(v:Vector3, u:Num):Num = StandardPaths.line(Vector3.zero, Vector3.one).applyOffset(v).at(u).x";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string rhs)[] ConcatenateArgs =
        {
            ("(0.5, 0)", "Vector3(0, 0, 0)"),
            ("(0.5, 0.4)", "Vector3(0.8, 0, 0)"),
            ("(0.5, 0.5)", "Vector3(1, 0, 0)"),
            ("(0.5, 0.9)", "Vector3(1, 0.8, 0)"),
            ("(0.5, 1)", "Vector3(1, 1, 0)"),
            ("(0.2, 0)", "Vector3(0, 0, 0)"),
            ("(0.2, 0.1)", "Vector3(0.5, 0, 0)"),
            ("(0.2, 0.2)", "Vector3(1, 0, 0)"),
            ("(0.2, 0.6)", "Vector3(1, 0.5, 0)"),
            ("(0.2, 1)", "Vector3(1, 1, 0)"),
        };
        [Test]
        public void Concatenate([ValueSource(nameof(ConcatenateArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction = "_(boundary:Num, u:Num):Vector3 = " +
                                  "Path.concatenate(" +
                                  "StandardPaths.line(Vector3(0, 0, 0), Vector3(1, 0, 0))," +
                                  "StandardPaths.line(Vector3(1, 0, 0), Vector3(1, 1, 0))," +
                                  "boundary).at(u)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
    }
}