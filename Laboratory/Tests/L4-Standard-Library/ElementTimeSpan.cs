using System;
using System.Linq;
using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class ElementTimeSpan : StandardLibraryFixture
    {

        // Most tests are written for the "fromSeconds" factory function, as the default constructor hides the implementation
        // detail of separating out the fractional and integer components.
        [
            TestCase("TimeSpan.fromSeconds(1).add(TimeSpan.fromSeconds(2))", "TimeSpan.fromSeconds(3)"),  // Integer seconds can be added 
            TestCase("TimeSpan.fromSeconds(1).add(TimeSpan.fromSeconds(4))", "TimeSpan.fromSeconds(5)"),
            TestCase("TimeSpan.fromSeconds(0.2).add(TimeSpan.fromSeconds(0.3))", "TimeSpan.fromSeconds(0.5)"),  // Fractional parts can be added 
            TestCase("TimeSpan.fromSeconds(0.7).add(TimeSpan.fromSeconds(0.8))", "TimeSpan.fromSeconds(1.5)"),
            TestCase("TimeSpan.fromSeconds(3.5).add(TimeSpan.fromSeconds(4.8))", "TimeSpan.fromSeconds(8.3)"),  // Complete numbers can be added
        ]
        public void AddPositive(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("TimeSpan.fromSeconds(-1).add(TimeSpan.fromSeconds(-3))", "TimeSpan.fromSeconds(-4)"),    // The integer parts can be added
            TestCase("TimeSpan.fromSeconds(-1.3).add(TimeSpan.fromSeconds(-1.2))", "TimeSpan.fromSeconds(-2.5)"),    // The fractional parts can be added
        ]
        public void AddNegative(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);



        [
            TestCase("TimeSpan.fromSeconds(10).sub(TimeSpan.fromSeconds(3.4))", "TimeSpan.fromSeconds(6.6)"),
            TestCase("TimeSpan.fromSeconds(10).sub(TimeSpan.fromSeconds(3.6))", "TimeSpan.fromSeconds(6.4)"),
            TestCase("TimeSpan.fromSeconds(1).sub(TimeSpan.fromSeconds(0.4))", "TimeSpan.fromSeconds(0.6)"),
            TestCase("TimeSpan.fromSeconds(1).sub(TimeSpan.fromSeconds(0.5))", "TimeSpan.fromSeconds(0.5)"),
            TestCase("TimeSpan.fromSeconds(1).sub(TimeSpan.fromSeconds(0.6))", "TimeSpan.fromSeconds(0.4)"),
            TestCase("TimeSpan.fromSeconds(1).sub(TimeSpan.fromSeconds(1))", "TimeSpan.fromSeconds(0)"),
            TestCase("TimeSpan.fromSeconds(1).sub(TimeSpan.fromSeconds(1.4))", "TimeSpan.fromSeconds(-0.4)"),
            TestCase("TimeSpan.fromSeconds(1).sub(TimeSpan.fromSeconds(1.6))", "TimeSpan.fromSeconds(-0.6)"),
            TestCase("TimeSpan.fromSeconds(1).sub(TimeSpan.fromSeconds(2.5))", "TimeSpan.fromSeconds(-1.5)"),
        ]
        public void Subtract(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase("TimeSpan.fromSeconds(2.3).mul(2)", "TimeSpan.fromSeconds(4.6)"),  // Numbers can be multiplied
            TestCase("TimeSpan.fromSeconds(2.7).mul(2)", "TimeSpan.fromSeconds(5.4)"),      // Fractional parts cannot exceed 1
            TestCase("TimeSpan.fromSeconds(2.72).mul(10)", "TimeSpan.fromSeconds(27.2)"),
        ]
        public void MulPositive(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("TimeSpan.fromSeconds(1.7).mul(-1)", "TimeSpan.fromSeconds(-1.7)"),    // Numbers can be multiplied by negative numbers
            TestCase("TimeSpan.fromSeconds(2.7).mul(-2)", "TimeSpan.fromSeconds(-5.4)"),
            TestCase("TimeSpan.fromSeconds(2.72).mul(-10)", "TimeSpan.fromSeconds(-27.2)"),
            TestCase("TimeSpan.fromSeconds(-2.3).mul(-2)", "TimeSpan.fromSeconds(4.6)"),
            TestCase("TimeSpan.fromSeconds(-2.7).mul(-2)", "TimeSpan.fromSeconds(5.4)"),
            TestCase("TimeSpan.fromSeconds(-2.72).mul(-10)", "TimeSpan.fromSeconds(27.2)"),

        ]
        public void MulNegative(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


        [
            TestCase("TimeSpan.fromSeconds(2.3).div(2)", "TimeSpan.fromSeconds(1.15)"),  // Numbers can be multiplied
            TestCase("TimeSpan.fromSeconds(2.7).div(2)", "TimeSpan.fromSeconds(1.35)"),      // Fractional parts cannot exceed 1
            TestCase("TimeSpan.fromSeconds(2.72).div(10)", "TimeSpan.fromSeconds(0.272)"),
            TestCase("TimeSpan.fromSeconds(1.7).div(-1)", "TimeSpan.fromSeconds(-1.7)"),    // Numbers can be multiplied by negative numbers
            TestCase("TimeSpan.fromSeconds(2.7).div(-2)", "TimeSpan.fromSeconds(-1.35)"),
            TestCase("TimeSpan.fromSeconds(2.72).div(-10)", "TimeSpan.fromSeconds(-0.272)"),
        ]
        public void Div(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("TimeSpan.fromSeconds(1).abs", "TimeSpan.fromSeconds(1)"),
            TestCase("TimeSpan.fromSeconds(1.5).abs", "TimeSpan.fromSeconds(1.5)"),
            TestCase("TimeSpan.fromSeconds(1.75).abs", "TimeSpan.fromSeconds(1.75)"),
            TestCase("TimeSpan.fromSeconds(2).abs", "TimeSpan.fromSeconds(2)"),
            TestCase("TimeSpan.fromSeconds(-1.5).abs", "TimeSpan.fromSeconds(1.5).abs"),
            TestCase("TimeSpan.fromSeconds(-1.75).abs", "TimeSpan.fromSeconds(1.75).abs"),
            TestCase("TimeSpan.fromSeconds(-2).abs", "TimeSpan.fromSeconds(2).abs"),
        ]
        public void Abs(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("TimeSpan.fromSeconds(1).negate", "TimeSpan.fromSeconds(-1)"),
            TestCase("TimeSpan.fromSeconds(1.5).negate", "TimeSpan.fromSeconds(-1.5)"),
            TestCase("TimeSpan.fromSeconds(1.75).negate", "TimeSpan.fromSeconds(-1.75)"),
            TestCase("TimeSpan.fromSeconds(2).negate", "TimeSpan.fromSeconds(-2)"),
            TestCase("TimeSpan.fromSeconds(-1).negate", "TimeSpan.fromSeconds(1)"),
            TestCase("TimeSpan.fromSeconds(-1.5).negate", "TimeSpan.fromSeconds(1.5)"),
            TestCase("TimeSpan.fromSeconds(-1.75).negate", "TimeSpan.fromSeconds(1.75)"),
            TestCase("TimeSpan.fromSeconds(-2).negate", "TimeSpan.fromSeconds(2)"),
        ]
        public void Negate(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);


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