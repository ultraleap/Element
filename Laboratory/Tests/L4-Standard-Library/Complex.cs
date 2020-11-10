using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Complex : StandardLibraryFixture
    {
        public static string ComplexConstructor = "_(x:Num, y:Num):Complex = Complex(x, y)";
        
        public static (string constructorArgs, string constant)[] ConstantArgList =
        {
            ("(0, 0)", "Complex.zero"),
            ("(1, 0)", "Complex.unitReal"),
            ("(0, 1)", "Complex.unitImag"),
        };
        [Test]
        public void Constants([ValueSource(nameof(ConstantArgList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(ComplexConstructor, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        
        public static (string fromPolarArgs, string constructorArgs)[] FromPolarArgsList =
        {
            ("(0, 0)", "(0, 0)"),
            ("(1, 0)", "(1, 0)"),
            ("(0, Num.pi)", "(0, 0)"),
            ("(2, Num.pi)", "(-2, 0)"),
            ("(2, -1.mul(Num.pi))", "(-2, 0)"),
            ("(Num.sqrt(2), Num.pi.div(4))", "(1, 1)"),
            ("(Num.sqrt(2), -1.mul(Num.pi.div(4)))", "(1, -1)"),
        };
        [Test]
        public void FromPolar([ValueSource(nameof(FromPolarArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqualRelaxed(ValidatedCompilerInput,
                new FunctionEvaluation("Complex.fromPolar", args.lhs, mode),
                new FunctionEvaluation(ComplexConstructor, args.rhs, mode));
        }
        
        public static (string constructorArgs, string property, string result)[] ComponentsArgsList =
        {
            ("(0, 0)", "real", "0"),
            ("(1, 0)", "real", "1"),
            ("(-18.4, 0.28)", "real", "-18.4"),
            ("(0, 0)", "imag", "0"),
            ("(1, 0)", "imag", "0"),
            ("(-18.4, 0.28)", "imag", "0.28"),
        };
        [Test]
        public void Components(
            [ValueSource(nameof(ComponentsArgsList))] (string constructorArgs, string property, string result) args,
            [Values] EvaluationMode mode
        )
        {
            string getter = "_(a:Num, b:Num):Num = Complex(a, b)." + args.property;
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.constructorArgs, mode),
                new ExpressionEvaluation(args.result, mode));
        }

        public static (string constructorArgs, string property, string result)[] PropertiesArgsList =
        {
            ("(0, 0)", "modulusSquared", "0"),
            ("(1, 0)", "modulusSquared", "1"),
            ("(0, 1)", "modulusSquared", "1"),
            ("(1, 1)", "modulusSquared", "2"),
            ("(3, -4)", "modulusSquared", "25"),
            ("(0, 0)", "modulus", "0"),
            ("(1, 0)", "modulus", "1"),
            ("(0, 1)", "modulus", "1"),
            ("(1, 1)", "modulus", "Num.sqrt(2)"),
            ("(3, -4)", "modulus", "5"),
            ("(1, 0)", "argument", "0"),
            ("(-1, -1)", "argument", "Num.pi.mul(-3.div(4))"),
            ("(0, 1)", "argument", "Num.pi.div(2)"),
            ("(0, -2)", "argument", "-1.mul(Num.pi.div(2))"),
        };
        [Test]
        public void Properties(
            [ValueSource(nameof(PropertiesArgsList))] (string constructorArgs, string property, string result) args,
            [Values] EvaluationMode mode
        )
        {
            string getter = "_(a:Num, b:Num):Num = Complex(a, b)." + args.property;
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.constructorArgs, mode),
                new ExpressionEvaluation(args.result, mode));
        }
        
        public static (string constructorArgs, string conjugateConstructorArgs)[] ConjugateArgsList =
        {
            ("(0, 0)", "(0, 0)"),
            ("(1, 0)", "(1, 0)"),
            ("(1, 1)", "(1, -1)"),
            ("(-1, 1)", "(-1, -1)"),
            ("(-1, -1)", "(-1, 1)"),
        };
        [Test]
        public void Conjugate(
            [ValueSource(nameof(ConjugateArgsList))] (string lhs, string rhs) args,
            [Values] EvaluationMode mode
        )
        {
            string getter = "_(a:Num, b:Num):Complex = Complex(a, b).conjugate";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getter, args.lhs, mode),
                new FunctionEvaluation(ComplexConstructor, args.rhs, mode));
        }

        public static (string lhs, string rhs)[] ComplexMulArgsList =
        {
            ("(Complex(0, 0), Complex(0, 0))", "Complex(0, 0)"),
            ("(Complex(1, 0), Complex(1, 0))", "Complex(1, 0)"),
            ("(Complex(1, 0), Complex(0, 1))", "Complex(0, 1)"),
            ("(Complex(0, 1), Complex(0, 1))", "Complex(-1, 0)"),
            ("(Complex(1, 1), Complex(1, 1))", "Complex(0, 2)"),
            ("(Complex(2, 1), Complex(2, -1))", "Complex(5, 0)"),
            ("(Complex(2, 1), Complex(2, -5))", "Complex(9, -8)"),
        };
        [Test]
        public void ComplexMul(
            [ValueSource(nameof(ComplexMulArgsList))] (string lhs, string rhs) args,
            [Values] EvaluationMode mode
        )
        {
            string complexMul = "_(u:Complex, v:Complex):Complex = u.mul(v)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(complexMul, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string rhs)[] ComplexDivArgsList =
        {
            ("(Complex(0, 0), Complex(0, 0))", "Complex(Num.NaN, Num.NaN)"),
            ("(Complex(1, 0), Complex(1, 0))", "Complex(1, 0)"),
            ("(Complex(1, 0), Complex(0, 1))", "Complex(0, -1)"),
            ("(Complex(0, 1), Complex(0, 1))", "Complex(1, 0)"),
            ("(Complex(1, 1), Complex(1, 1))", "Complex(1, 0)"),
            ("(Complex(2, 1), Complex(2, -1))", "Complex(3.div(5), 4.div(5))"),
            ("(Complex(2, 1), Complex(2, -5))", "Complex(-1.div(29), 12.div(29))"),
        };
        [Test]
        public void ComplexDiv(
            [ValueSource(nameof(ComplexDivArgsList))] (string lhs, string rhs) args,
            [Values] EvaluationMode mode
        )
        {
            string complexMul = "_(u:Complex, v:Complex):Complex = u.div(v)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(complexMul, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        public static (string lhs, string op, string rhs)[] ComplexAddSubArgsList =
        {
            ("(Complex(0, 0), Complex(0, 0))", "add", "Complex(0, 0)"),
            ("(Complex(1, 2), Complex(0, 0))", "add", "Complex(1, 2)"),
            ("(Complex(4, 7), Complex(1, -5))", "add", "Complex(5, 2)"),
            ("(Complex(-8, -7.2), Complex(1, 5.2))", "add", "Complex(-7, -2)"),
            ("(Complex(0, 0), Complex(0, 0))", "sub", "Complex(0, 0)"),
            ("(Complex(1, 2), Complex(0, 0))", "sub", "Complex(1, 2)"),
            ("(Complex(4, 7), Complex(1, -5))", "sub", "Complex(3, 12)"),
            ("(Complex(-8, -7.2), Complex(1, 5.2))", "sub", "Complex(-9, -12.4)"),
        };
        [Test]
        public void ComplexAddSub(
            [ValueSource(nameof(ComplexAddSubArgsList))] (string lhs, string op, string rhs) args,
            [Values] EvaluationMode mode
        )
        {
            string complexMul = "_(u:Complex, v:Complex):Complex = u." + args.op + "(v)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(complexMul, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
        
        
        public static (string lhs, string rhs)[] ComplexScaleArgsList =
        {
            ("(Complex(0, 0), 1)", "Complex(0, 0)"),
            ("(Complex(13, 91), 0)", "Complex(0, 0)"),
            ("(Complex(2, 1), 1)", "Complex(2, 1)"),
            ("(Complex(-3, 4), -2)", "Complex(6, -8)"),
        };
        [Test]
        public void ComplexScale(
            [ValueSource(nameof(ComplexScaleArgsList))] (string lhs, string rhs) args,
            [Values] EvaluationMode mode
        )
        {
            string complexScale = "_(c:Complex, n:Num):Complex = c.scale(n)";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(complexScale, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }
    }
}
/**
MandelbrotIterationState -> iteration:num, z:Complex;

Mandelbrot(x:num, y:num, width:num, height:num, maxIterations:num, maxExtent:num):num
{
scale = mul(2, div(maxExtent, min(width, height)));
c = Complex(mul(sub(x, div(width, 2)), scale),
            mul(sub(div(height, 2), y), scale));

state = MandelbrotIterationState(0, Complex(0, 0));
condition(state) = and(lt(state.iteration, maxIterations), lt(ComplexNorm(state.z), sqr(maxExtent)));
body(state) = MandelbrotIterationState(
    add(state.iteration, 1),
    ComplexAdd(ComplexMul(state.z, state.z), c)
);
result = for(state, condition, body);
return = if(lt(result.iteration, maxIterations), div(result.iteration, maxIterations), 0);
}
 */
 