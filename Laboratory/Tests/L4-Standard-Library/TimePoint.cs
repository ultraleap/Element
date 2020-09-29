using System;
using System.Linq;
using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class TimePoint : StandardLibraryFixture
    {
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