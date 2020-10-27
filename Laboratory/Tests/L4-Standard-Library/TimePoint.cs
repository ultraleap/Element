using System;
using System.Linq;
using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class TimePoint : StandardLibraryFixture
    {

        // Most tests are written for the "fromSeconds" factory function, as the default constructor hides the implementation
        // detail of separating out the fractional and integer components.
        [
            TestCase("TimePoint.fromSeconds(1).add(TimePoint.fromSeconds(2))", "TimePoint.fromSeconds(3)"),  // Integer seconds can be added 
            TestCase("TimePoint.fromSeconds(1).add(TimePoint.fromSeconds(4))", "TimePoint.fromSeconds(5)"),
            TestCase("TimePoint.fromSeconds(0.2).add(TimePoint.fromSeconds(0.3))", "TimePoint.fromSeconds(0.5)"),  // Fractional parts can be added 
            TestCase("TimePoint.fromSeconds(0.7).add(TimePoint.fromSeconds(0.8))", "TimePoint.fromSeconds(1.5)"),
            TestCase("TimePoint.fromSeconds(3.5).add(TimePoint.fromSeconds(4.8))", "TimePoint.fromSeconds(8.3)"),  // Complete numbers can be added
        ]
        public void AddPositive(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("TimePoint.fromSeconds(-1).add(TimePoint.fromSeconds(-3))", "TimePoint.fromSeconds(-4)"),    // The integer parts can be added
            TestCase("TimePoint.fromSeconds(-1.3).add(TimePoint.fromSeconds(-1.2))", "TimePoint.fromSeconds(-2.5)"),    // The fractional parts can be added
        ]
        public void AddNegative(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("TimePoint.fromSeconds(2.3).mul(2)", "TimePoint.fromSeconds(4.6)"),  // Numbers can be multiplied
            TestCase("TimePoint.fromSeconds(2.7).mul(2)", "TimePoint.fromSeconds(5.4)"),      // Fractional parts cannot exceed 1
            TestCase("TimePoint.fromSeconds(2.72).mul(10)", "TimePoint.fromSeconds(27.2)"),
        ]
        public void MulPositive(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("TimePoint.fromSeconds(1.7).mul(-1)", "TimePoint.fromSeconds(-1.7)"),    // Numbers can be multiplied by negative numbers
            TestCase("TimePoint.fromSeconds(2.7).mul(-2)", "TimePoint.fromSeconds(-5.4)"),
            TestCase("TimePoint.fromSeconds(2.72).mul(-10)", "TimePoint.fromSeconds(-27.2)"),

        ]
        public void MulNegative(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("TimePoint.fromSeconds(1).abs", "TimePoint.fromSeconds(1)"),
            TestCase("TimePoint.fromSeconds(1.5).abs", "TimePoint.fromSeconds(1.5)"),
            TestCase("TimePoint.fromSeconds(1.75).abs", "TimePoint.fromSeconds(1.75)"),
            TestCase("TimePoint.fromSeconds(2).abs", "TimePoint.fromSeconds(2)"),
            TestCase("TimePoint.fromSeconds(-1.5).abs", "TimePoint.fromSeconds(1.5).abs"),
            TestCase("TimePoint.fromSeconds(-1.75).abs", "TimePoint.fromSeconds(1.75).abs"),
            TestCase("TimePoint.fromSeconds(-2).abs", "TimePoint.fromSeconds(2).abs"),
        ]
        public void Abs(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilerInput, expected, expression);

        [
            TestCase("TimePoint.fromSeconds(1).negate", "TimePoint.fromSeconds(-1)"),
            TestCase("TimePoint.fromSeconds(1.5).negate", "TimePoint.fromSeconds(-1.5)"),
            TestCase("TimePoint.fromSeconds(1.75).negate", "TimePoint.fromSeconds(-1.75)"),
            TestCase("TimePoint.fromSeconds(2).negate", "TimePoint.fromSeconds(-2)"),
            TestCase("TimePoint.fromSeconds(-1).negate", "TimePoint.fromSeconds(1)"),
            TestCase("TimePoint.fromSeconds(-1.5).negate", "TimePoint.fromSeconds(1.5)"),
            TestCase("TimePoint.fromSeconds(-1.75).negate", "TimePoint.fromSeconds(1.75)"),
            TestCase("TimePoint.fromSeconds(-2).negate", "TimePoint.fromSeconds(2)"),
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

        // TODO: Test that TimePoint.value returns comparable interval of values for periods at large offsets
        // e.g. value(n) = n*period...n*period+period == n*period+offset...n*period+offset+period should return same values

        /*[TestCaseSource(nameof(_args))]
        public void TimePointStability((int FrameIndex, int SampleRate, float Period) args)
        {
            float EvalWithHost(Element.CLR.TimePoint t, float period) =>
                Host.EvaluateExpression(ValidatedCompilerInput, $"TimePoint.value(TimePoint({t.whole}, {t.fractional}), {period})")
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

            float EvalDirectly(Element.CLR.TimePoint t, float period) => (t.whole % period) + (t.fractional % period);

            var frameIndexes = Enumerable.Range(args.FrameIndex, 5).ToArray();
            var samplesToCheck = frameIndexes.Select(i => new Element.CLR.TimePoint((double)i / args.SampleRate)).ToArray();

            var hostResults = samplesToCheck.Select(t => EvalWithHost(t, args.Period)).ToArray();
            var directResults = samplesToCheck.Select(t => EvalDirectly(t, args.Period)).ToArray();
            CollectionAssert.AreEqual(hostResults, directResults, "Host and direct results are different");

            var actualRoundTrippedResults = hostResults.Select(r => (int)Math.Round(r * args.SampleRate));
            var expectedRoundTripResults = frameIndexes.Select(i => i % (int) (args.Period * args.SampleRate));
            CollectionAssert.AreEqual(actualRoundTrippedResults, expectedRoundTripResults);
        }
        
        [TestCaseSource(nameof(_args))]
        public void TimePointStabilityWithoutPeriod((int FrameIndex, int SampleRate, float Period) args)
        {
            float EvalWithHost(Element.CLR.TimePoint t, float period) =>
                Host.EvaluateExpression(ValidatedCompilerInput, $"TimePoint.value(TimePoint({t.whole}, {t.fractional}), {period})")
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

            float EvalDirectly(Element.CLR.TimePoint t, float period) => (t.whole % period) + (t.fractional % period);

            var frameIndexes = Enumerable.Range(args.FrameIndex, 5).ToArray();
            var samplesToCheck = frameIndexes.Select(i => new Element.CLR.TimePoint((double)i / args.SampleRate)).ToArray();

            var hostResults = samplesToCheck.Select(t => EvalWithHost(t, args.Period)).ToArray();
            var directResults = samplesToCheck.Select(t => EvalDirectly(t, args.Period)).ToArray();
            CollectionAssert.AreEqual(hostResults, directResults, "Host and direct results are different");

            var roundTrippedResults = hostResults.Select(r => (int)Math.Round(r * args.SampleRate));
            CollectionAssert.AreEqual(roundTrippedResults, frameIndexes.Select(i => i % (int)(args.Period * args.SampleRate)));
        }*/
    }
}