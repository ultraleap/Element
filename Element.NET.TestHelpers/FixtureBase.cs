using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using NUnit.Framework;

namespace Element.NET.TestHelpers
{
    [TestFixture, Parallelizable(ParallelScope.All)]
    public abstract class FixtureBase
    {
        public const float FloatEpsilon = 1.19209e-5f;
        protected static Comparer<float> FloatComparer { get; } = Comparer<float>.Create((f, f1) => ApproximatelyEqualEpsilon(f, f1, FloatEpsilon) ? 0 : 1);
        protected static Comparer<float> RelaxedFloatComparer { get; } = Comparer<float>.Create((f, f1) => Math.Abs(f - f1) < FloatEpsilon ? 0 : 1);

        // Taken from https://stackoverflow.com/questions/3874627/floating-point-comparison-functions-for-c-sharp
        public static bool ApproximatelyEqualEpsilon(float a, float b, float epsilon)
        {
            const float floatNormal = (1 << 23) * float.Epsilon;
            float absA = Math.Abs(a);
            float absB = Math.Abs(b);
            float diff = Math.Abs(a - b);

            // ReSharper disable CompareOfFloatsByEqualityOperator
            if (a == b)
            {
                // Shortcut, handles infinities
                return true;
            }
            
            if (float.IsNaN(a) && float.IsNaN(b)) return true;

            if (a == 0.0f || b == 0.0f || diff < floatNormal)
            {    
                // a or b is zero, or both are extremely close to it.
                // relative error is less meaningful here
                return diff < (epsilon * floatNormal);
            }

            // use relative error
            return diff / Math.Min(absA + absB, float.MaxValue) < epsilon;
            // ReSharper enable CompareOfFloatsByEqualityOperator
        }
        
        protected static void LogMessages(IEnumerable<CompilerMessage> messages)
        {
            foreach (var message in messages)
            {
                TestContext.WriteLine(message.ToString());
            }
        }

        private static string NameOrUnknown(int? messageCode) => NameOrUnknown(messageCode.HasValue ? (MessageCode?)messageCode.Value : null);
        private static string NameOrUnknown(MessageCode? messageCode) => messageCode.HasValue
                                                                             ? CompilerMessage.TryGetMessageName(messageCode.Value, out var name) ? name! : "?"
                                                                             : "CustomError";
        
        protected static void ExpectingError(IReadOnlyCollection<CompilerMessage> messages, bool success, MessageCode messageCode)
        {
            var errors = messages.Where(s => s.MessageLevel >= MessageLevel.Error && s.MessageCode.HasValue).ToArray();
            var hasErrors = errors.Any();
            LogMessages(messages);
            
            if (messages.Any(s => s.MessageCode == (int)messageCode)) 
                Assert.Pass($"Received expected message code {messageCode}");
            
            if (success) 
                Assert.Fail("Expected error ELE{0} '{1}' but succeeded",
                    (int)messageCode, NameOrUnknown(messageCode));

            if (hasErrors) 
                Assert.Fail("Expected error ELE{0} '{1}' but got following errors instead: {2}",
                    (int)messageCode, NameOrUnknown(messageCode),
                    string.Join(", ", errors.Select(err => NameOrUnknown(err.MessageCode))));
            
            Assert.Fail("Expected message code {0} '{1}', but success was false and no errors were received", (int)messageCode, NameOrUnknown(messageCode));
        }
        
        protected static void ExpectingSuccess(IReadOnlyCollection<CompilerMessage> messages, bool success)
        {
            var errors = messages.Where(s => s.MessageLevel >= MessageLevel.Error).ToArray();
            var hasErrors = errors.Any();
            LogMessages(messages);
            
            if (hasErrors) 
                Assert.Fail("Expected success and got following error(s): {0}", 
                    string.Join(",", errors.Select(err => NameOrUnknown(err.MessageCode))));
            
            if(success)
                Assert.Pass("Received success");
            else
                Assert.Fail("Received failure");
        }
        
        protected static DirectoryInfo TestDirectory { get; } = new DirectoryInfo(Directory.GetCurrentDirectory());
        protected static DirectoryPackageRegistry TestPackageRegistry => new DirectoryPackageRegistry(TestDirectory);
        protected static FileInfo GetEleFile(string partialName) => GetFile("*.ele", partialName);
        protected static FileInfo GetFile(string pattern, string partialName) => TestDirectory.GetFiles(pattern, SearchOption.AllDirectories).Single(file => file.Name.Contains(partialName));

        protected static SourceContext MakeSourceContext(CompilerOptions compilerOptions = default, string extraSource = default) =>
            SourceContext.CreateAndLoad(new CompilerInput(TestPackageRegistry,
                                                          Array.Empty<PackageSpecifier>(),
                                                         Array.Empty<FileInfo>(),
                                                          compilerOptions))
                         .Then(context => string.IsNullOrEmpty(extraSource)
                                              ? context // Do nothing if extra source is empty
                                              : context.LoadElementSource(new SourceInfo("ExtraTestSource", extraSource)))
                         .Match((context, messages) =>
                         {
                             LogMessages(messages);
                             return context;
                         }, messages =>
                         {
                             LogMessages(messages);
                             Assert.Fail("Failed to create source context");
                             return null!; // This will never be reached
                         });
    }
}