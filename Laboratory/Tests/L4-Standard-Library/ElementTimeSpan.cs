using System;
using System.Linq;
using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class ElementTimeSpan : StandardLibraryFixture
    {
        // Most tests are written for the "fromSeconds" factory function, as the default constructor hides the implementation
        // detail of separating out the fractional and integer components.
        public static string TimeSpanFromSeconds = "_(s:Num):TimeSpan = TimeSpan.fromSeconds(s)";

        public static (string TimeSpanArgs, string Property, string Period, string Output)[] PropertiesArgsList = 
        {
            ("0, 0", "cycles", "1",  "0"),
            ("0, 0.9", "cycles", "1",  "0"),
            ("5, 0", "cycles", "1",  "5"),
            ("5, 0.9", "cycles", "1",  "5"),
            ("5, 0", "cycles", "5",  "1"),
            ("4, 0.9", "cycles", "5",  "0"),
            ("0, 0.9", "cycles", "0.2",  "4"),
            ("10, 0.9", "cycles", "0.2",  "54"),
            ("0, 0", "value", "1",  "0"),
            ("0, 0.9", "value", "1",  "0.9"),
            ("5, 0", "value", "1",  "0"),
            ("5, 0.9", "value", "1",  "0.9"),
            ("5, 0", "value", "5",  "0"),
            ("4, 0.9", "value", "5",  "4.9"),
            ("0, 0.9", "value", "0.2",  "0.1"),
            ("10, 0.9", "value", "0.2",  "0.1"),
            ("1, 0", "progress", "1", "0"),
        };

        [Test]
        public void GetProperties([ValueSource(nameof(PropertiesArgsList))] (string tsargs, string property, string period, string output) args,
            [Values] EvaluationMode mode)
        {
            string getPropertyArgs = "(" + args.tsargs + ", " + args.period + ")";
            string getProperty = "_(a:Num, b:Num, p:Num):Num = TimeSpan(a, b)." + args.property + "(" + args.period + ")";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(getProperty, getPropertyArgs, mode),
                new ExpressionEvaluation(args.output, mode));
        }

        public static string TimeSpanAdder = "_(a:Num, b:Num):TimeSpan = TimeSpan.fromSeconds(a).add(TimeSpan.fromSeconds(b))";
        public static (string LeftHandArgs, string RightHandArgs)[] AddArgsList =
        {
            ("(1, 2)", "(3)"), // Integer seconds can be added
            ("(1, 4)", "(5)"),
            ("(0.2, 0.3)", "(0.5)"), // Fractional parts can be added
            ("(0.7, 0.8)", "(1.5)"),
            ("(3.5, 4.8)", "(8.3)"), // Complete numbers can be added
            ("(-1, -3)", "(-4)"),    // The integer parts can be added
            ("(-1.3, -1.2)", "(-2.5)"), // The fractional parts can be added
        };

        [Test]
        public void AddTimeSpans([ValueSource(nameof(AddArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(TimeSpanAdder, args.lhs, mode),
                              new FunctionEvaluation(TimeSpanFromSeconds, args.rhs, mode));
        }


        public static string TimeSpanSubtractor = "_(a:Num, b:Num):TimeSpan = TimeSpan.fromSeconds(a).sub(TimeSpan.fromSeconds(b))";
        public static (string LeftHandArgs, string RightHandArgs)[] SubArgsList =
        {
            ("(10, 3.4)", "(6.6)"),
            ("(10, 3.6)", "(6.4)"),
            ("(1, 0.4)", "(0.6)"),
            ("(1, 0.5)", "(0.5)"),
            ("(1, 0.6)", "(0.4)"),
            ("(1, 1)", "(0)"),
            ("(1, 1.4)", "(-0.4)"),
            ("(1, 1.6)", "(-0.6)"),
            ("(1, 2.5)", "(-1.5)"),
        };

        [Test]
        public void SubtractTimeSpans([ValueSource(nameof(SubArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(TimeSpanSubtractor, args.lhs, mode),
                              new FunctionEvaluation(TimeSpanFromSeconds, args.rhs, mode));
        }


        public static string TimeSpanMultiplier = "_(a:Num, b:Num):TimeSpan = TimeSpan.fromSeconds(a).mul(b)";
        public static (string LeftHandArgs, string RightHandArgs)[] MulArgsList =
        {
            ("(2.3, 2)", "(4.6)"),
            ("(2.7, 2)", "(5.4)"),
            ("(2.72, 10)", "(27.2)"),
            ("(1.7, -1)", "(-1.7)"),
            ("(2.7, -2)", "(-5.4)"),
            ("(2.72, -10)", "(-27.2)"),
            ("(-2.3, -2)", "(4.6)"),
            ("(-2.7, -2)", "(5.4)"),
            ("(-2.72, -10)", "(27.2)"),
        };

        [Test]
        public void MultiplyTimeSpanByNumber([ValueSource(nameof(MulArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(TimeSpanMultiplier, args.lhs, mode),
                              new FunctionEvaluation(TimeSpanFromSeconds, args.rhs, mode));
        }


        public static string TimeSpanDivider = "_(a:Num, b:Num):TimeSpan = TimeSpan.fromSeconds(a).div(b)";
        public static (string LeftHandArgs, string RightHandArgs)[] DivArgsList =
        {
            ("(2.3, 2)", "(1.15)"),
            ("(2.7, 2)", "(1.35)"),
            ("(2.72, 10)", "(0.272)"),
            ("(1.7, -1)", "(-1.7)"),
            ("(2.7, -2)", "(-1.35)"),
            ("(2.72, -10)", "(-0.272)"),
        };

        [Test]
        public void DivideTimeSpanByNumber([ValueSource(nameof(DivArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(TimeSpanDivider, args.lhs, mode),
                              new FunctionEvaluation(TimeSpanFromSeconds, args.rhs, mode));
        }

        public static string TimeSpanAbsoluter = "_(a:Num):TimeSpan = TimeSpan.fromSeconds(a).abs";
        public static (string LeftHandArgs, string RightHandArgs)[] AbsArgsList =
        {
            ("(1)", "(1)"),
            ("(1.5)", "(1.5)"),
            ("(1.75)", "(1.75)"),
            ("(2)", "(2)"),
            ("(-1.5)", "(1.5)"),
            ("(-1.75)", "(1.75)"),
            ("(-2)", "(2)"),
        };

        [Test]
        public void AbsoluteTimeSpan([ValueSource(nameof(AbsArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(TimeSpanAbsoluter, args.lhs, mode),
                              new FunctionEvaluation(TimeSpanFromSeconds, args.rhs, mode));
        }

        public static string TimeSpanNegator = "_(a:Num):TimeSpan = TimeSpan.fromSeconds(a).negate";
        public static (string LeftHandArgs, string RightHandArgs)[] NegatorArgsList =
        {
            ("(1)", "(-1)"),
            ("(1.5)", "(-1.5)"),
            ("(1.75)", "(-1.75)"),
            ("(2)", "(-2)"),
            ("(-1)", "(1)"),
            ("(-1.5)", "(1.5)"),
            ("(-1.75)", "(1.75)"),
            ("(-2)", "(2)"),
        };

        [Test]
        public void NegateTimeSpan([ValueSource(nameof(NegatorArgsList))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            AssertApproxEqual(ValidatedCompilerInput,
                              new FunctionEvaluation(TimeSpanNegator, args.lhs, mode),
                              new FunctionEvaluation(TimeSpanFromSeconds, args.rhs, mode));
        }
        
        public static (string fromSecondsArgs, string defaultConstructorArgs)[] FromSecondsConstructorArgs =
        {
            ("(0)", "(0, 0)"),
            ("(0.5)", "(0, 0.5)"),
            ("(1.1)", "(1, 0.1)"),
            ("(1.8)", "(1, 0.8)"),
        };

        [Test]
        public void FromSecondsConstructor([ValueSource(nameof(FromSecondsConstructorArgs))] (string lhs, string rhs) args, [Values] EvaluationMode mode)
        {
            string fromSecondsConstructor = "_(s:Num):TimeSpan = TimeSpan.fromSeconds(s)";
            string defaultConstructor = "_(a:Num, b:Num):TimeSpan = TimeSpan(a, b)";
                
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(fromSecondsConstructor, args.lhs, mode),
                new FunctionEvaluation(defaultConstructor, args.rhs, mode));
        }
        
        
        public static (string seconds, string op, string expected)[] ComparisonArgsList =
        {
            ("(1, 1)", "eq", "true"),
            ("(1, 1)", "neq", "false"),
            ("(1, 1)", "lt", "false"),
            ("(1, 1)", "leq", "true"),
            ("(1, 1)", "gt", "false"),
            ("(1, 1)", "geq", "true"),
            ("(1.3, 1.3)", "eq", "true"),
            ("(1.3, 1.3)", "neq", "false"),
            ("(1.3, 1.3)", "lt", "false"),
            ("(1.3, 1.3)", "leq", "true"),
            ("(1.3, 1.3)", "gt", "false"),
            ("(1.3, 1.3)", "geq", "true"),
            ("(-1.3, -1.3)", "eq", "true"),
            ("(-1.3, -1.3)", "neq", "false"),
            ("(-1.3, -1.3)", "lt", "false"),
            ("(-1.3, -1.3)", "leq", "true"),
            ("(-1.3, -1.3)", "gt", "false"),
            ("(-1.3, -1.3)", "geq", "true"),
            ("(1.1, 1.3)", "eq", "false"),
            ("(1.1, 1.3)", "neq", "true"),
            ("(1.1, 1.3)", "lt", "true"),
            ("(1.1, 1.3)", "leq", "true"),
            ("(1.1, 1.3)", "gt", "false"),
            ("(1.1, 1.3)", "geq", "false"),
            ("(1.1, 2)", "eq", "false"),
            ("(1.1, 2)", "neq", "true"),
            ("(1.1, 2)", "lt", "true"),
            ("(1.1, 2)", "leq", "true"),
            ("(1.1, 2)", "gt", "false"),
            ("(1.1, 2)", "geq", "false"),
            ("(2, 1.1)", "eq", "false"),
            ("(2, 1.1)", "neq", "true"),
            ("(2, 1.1)", "lt", "false"),
            ("(2, 1.1)", "leq", "false"),
            ("(2, 1.1)", "gt", "true"),
            ("(2, 1.1)", "geq", "true"),
            ("(1.8, 1.1)", "eq", "false"),
            ("(1.8, 1.1)", "neq", "true"),
            ("(1.8, 1.1)", "lt", "false"),
            ("(1.8, 1.1)", "leq", "false"),
            ("(1.8, 1.1)", "gt", "true"),
            ("(1.8, 1.1)", "geq", "true"),
            ("(-1, -1)", "eq", "true"),
            ("(-1, -1)", "neq", "false"),
            ("(-1, -1)", "lt", "false"),
            ("(-1, -1)", "leq", "true"),
            ("(-1, -1)", "gt", "false"),
            ("(-1, -1)", "geq", "true"),
            ("(-1, -1.5)", "eq", "false"),
            ("(-1, -1.5)", "neq", "true"),
            ("(-1, -1.5)", "lt", "false"),
            ("(-1, -1.5)", "leq", "false"),
            ("(-1, -1.5)", "gt", "true"),
            ("(-1, -1.5)", "geq", "true"),
            ("(-1, -2.5)", "eq", "false"),
            ("(-1, -2.5)", "neq", "true"),
            ("(-1, -2.5)", "lt", "false"),
            ("(-1, -2.5)", "leq", "false"),
            ("(-1, -2.5)", "gt", "true"),
            ("(-1, -2.5)", "geq", "true"),
            ("(-1.5, -1)", "eq", "false"),
            ("(-1.5, -1)", "neq", "true"),
            ("(-1.5, -1)", "lt", "true"),
            ("(-1.5, -1)", "leq", "true"),
            ("(-1.5, -1)", "gt", "false"),
            ("(-1.5, -1)", "geq", "false"),
            ("(-2.5, -1)", "eq", "false"),
            ("(-2.5, -1)", "neq", "true"),
            ("(-2.5, -1)", "lt", "true"),
            ("(-2.5, -1)", "leq", "true"),
            ("(-2.5, -1)", "gt", "false"),
            ("(-2.5, -1)", "geq", "false"),
        };
        [Test]
        public void Comparison([ValueSource(nameof(ComparisonArgsList))] (string lhs, string op, string rhs) args, [Values] EvaluationMode mode)
        {
            string testFunction =
                "_(a:Num, b:Num):Bool = TimeSpan." + args.op + "(TimeSpan.fromSeconds(a), TimeSpan.fromSeconds(b))";
            AssertApproxEqual(ValidatedCompilerInput,
                new FunctionEvaluation(testFunction, args.lhs, mode),
                new ExpressionEvaluation(args.rhs, mode));
        }

        private static (int FrameIndex, int SampleRate, float Period)[] _args =
            new[]
                {
                    0.001f,
                    0.01f,
                    0.1f,
                    0.5f,
                    1f,
                    10f,
                    60f
                }.SelectMany(period =>
                                 new[]
                                 {
                                     1000,
                                     10000,
                                     20000,
                                     40000,
                                     100000,
                                 }.SelectMany(sampleRate =>
                                                  new[]
                                                  {
                                                      TimeSpan.FromSeconds(5),
                                                      TimeSpan.FromMinutes(1),
                                                      TimeSpan.FromMinutes(5),
                                                      TimeSpan.FromMinutes(30),
                                                      TimeSpan.FromHours(1),
                                                      TimeSpan.FromHours(6),
                                                      TimeSpan.FromDays(1),
                                                  }.Select(time => ((int)(time.TotalSeconds * sampleRate), sampleRate, period))))
                 .ToArray();

        // TODO: Test that TimeSpan.value returns comparable interval of values for periods at large offsets
        // e.g. value(n) = n*period...n*period+period == n*period+offset...n*period+offset+period should return same values

        /*[TestCaseSource(nameof(_args))]
        public void TimeSpanStability((int FrameIndex, int SampleRate, float Period) args)
        {
            float EvalWithHost(Element.CLR.TimeSpan t, float period) =>
                Host.EvaluateExpression(ValidatedCompilerInput, $"TimeSpan.value(TimeSpan({t.whole}, {t.fractional}), {period})")
                    .Match((floats, messages) =>
                    {
                        LogMessages(messages);
                        return floats[0];
                    }, messages =>
                    {
                        LogMessages(messages);
                        Assert.Fail();
                        return float.NaN; // Will never reach here
                    });

            float EvalDirectly(Element.CLR.TimeSpan t, float period) => (t.whole % period) + (t.fractional % period);

            var frameIndexes = Enumerable.Range(args.FrameIndex, 5).ToArray();
            var samplesToCheck = frameIndexes.Select(i => new Element.CLR.TimeSpan((double)i / args.SampleRate)).ToArray();

            var hostResults = samplesToCheck.Select(t => EvalWithHost(t, args.Period)).ToArray();
            var directResults = samplesToCheck.Select(t => EvalDirectly(t, args.Period)).ToArray();
            CollectionAssert.AreEqual(hostResults, directResults, "Host and direct results are different");

            var actualRoundTrippedResults = hostResults.Select(r => (int)Math.Round(r * args.SampleRate));
            var expectedRoundTripResults = frameIndexes.Select(i => i % (int) (args.Period * args.SampleRate));
            CollectionAssert.AreEqual(actualRoundTrippedResults, expectedRoundTripResults);
        }
        
        [TestCaseSource(nameof(_args))]
        public void TimeSpanStabilityWithoutPeriod((int FrameIndex, int SampleRate, float Period) args)
        {
            float EvalWithHost(Element.CLR.TimeSpan t, float period) =>
                Host.EvaluateExpression(ValidatedCompilerInput, $"TimeSpan.value(TimeSpan({t.whole}, {t.fractional}), {period})")
                    .Match((floats, messages) =>
                    {
                        LogMessages(messages);
                        return floats[0];
                    }, messages =>
                    {
                        LogMessages(messages);
                        Assert.Fail();
                        return float.NaN; // Will never reach here
                    });

            float EvalDirectly(Element.CLR.TimeSpan t, float period) => (t.whole % period) + (t.fractional % period);

            var frameIndexes = Enumerable.Range(args.FrameIndex, 5).ToArray();
            var samplesToCheck = frameIndexes.Select(i => new Element.CLR.TimeSpan((double)i / args.SampleRate)).ToArray();

            var hostResults = samplesToCheck.Select(t => EvalWithHost(t, args.Period)).ToArray();
            var directResults = samplesToCheck.Select(t => EvalDirectly(t, args.Period)).ToArray();
            CollectionAssert.AreEqual(hostResults, directResults, "Host and direct results are different");

            var roundTrippedResults = hostResults.Select(r => (int)Math.Round(r * args.SampleRate));
            CollectionAssert.AreEqual(roundTrippedResults, frameIndexes.Select(i => i % (int)(args.Period * args.SampleRate)));
        }*/
    }
}