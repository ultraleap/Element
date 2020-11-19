using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using NUnit.Framework;

namespace Element.NET.TestHelpers
{
    [TestFixture, Parallelizable(ParallelScope.Fixtures)]
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

        private static string NameOrUnknown(string messageType, int? messageCode) => messageCode.HasValue
                                                                                        ? CompilerMessage.TryGetMessageName(messageType, messageCode.Value, out var name) ? name! : "?"
                                                                                        : "CustomError";

        protected static void ExpectingElementError(IReadOnlyCollection<CompilerMessage> messages, bool success, EleMessageCode eleMessageCode) =>
            ExpectingError(messages, success, MessageExtensions.TypeString, (int) eleMessageCode);
        
        protected static void ExpectingError(IReadOnlyCollection<CompilerMessage> messages, bool success, string messageType, int messageCode)
        {
            var errors = messages.Where(s => s.MessageLevel >= MessageLevel.Error).ToArray();
            var hasErrors = errors.Any();
            LogMessages(messages);
            
            if (messages.Any(s => s.MessageCode == messageCode)) 
                Assert.Pass("Received expected error {0}{1} '{2}'\n", messageType, messageCode, NameOrUnknown(messageType, messageCode));
            
            if (success)
                Assert.Fail("Expected error {0}{1} '{2}' but succeeded\n",
                            messageType, messageCode, NameOrUnknown(messageType, messageCode));

            if (hasErrors)
                Assert.Fail("Expected error {0}{1} '{2}' but got following errors instead: {3}\n",
                            messageType, messageCode, NameOrUnknown(messageType, messageCode),
                            string.Join(", ", errors.Select(err => NameOrUnknown(err.MessageType, err.MessageCode))));
            
            Assert.Fail("Expected {0}{1} '{2}', but success was false and no errors were received\n", messageType, messageCode, NameOrUnknown(messageType, messageCode));
        }
        
        protected static void ExpectingSuccess(IReadOnlyCollection<CompilerMessage> messages, bool success)
        {
            var errors = messages.Where(s => s.MessageLevel >= MessageLevel.Error).ToArray();
            var hasErrors = errors.Any();
            LogMessages(messages);
            
            if (hasErrors) 
                Assert.Fail("Expected success and got following error(s): {0}\n", 
                    string.Join(",", errors.Select(err => NameOrUnknown(err.MessageType, err.MessageCode))));
            
            if(success)
                Assert.Pass("Received success\n");
            else
                Assert.Fail("Received failure\n");
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
                                              ? Result.Success
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