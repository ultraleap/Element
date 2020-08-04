using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using NUnit.Framework;

namespace Element.NET.Tests
{
    [TestFixture, Parallelizable(ParallelScope.All)]
    public abstract class FixtureBase
    {
        protected static void LogMessages(IEnumerable<CompilerMessage> messages)
        {
            foreach (var message in messages)
            {
                TestContext.WriteLine(message.ToString());
            }
        }

        private static string NameOrUnknown(MessageCode messageCode) => CompilerMessage.TryGetMessageName(messageCode, out var name) ? name! : "?";
        
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
                Assert.Fail("Expected error ELE{0} '{1}' but got following error codes instead: {2}",
                    (int)messageCode, NameOrUnknown(messageCode),
                    string.Join(", ", errors.Select(err => $"ELE{err.MessageCode!.Value} {NameOrUnknown((MessageCode)err.MessageCode!.Value)}")));
            
            Assert.Fail("Expected message code {0} '{1}', but success was false and no errors were received", (int)messageCode, NameOrUnknown(messageCode));
        }
        
        protected static void ExpectingSuccess(IReadOnlyCollection<CompilerMessage> messages, bool success)
        {
            var errors = messages.Where(s => s.MessageLevel >= MessageLevel.Error).ToArray();
            var hasErrors = errors.Any();
            LogMessages(messages);
            
            if (hasErrors) 
                Assert.Fail("Expected success and got following code(s): {0}", 
                    string.Join(",", errors.Select(err => $"ELE{err.MessageCode!.Value} {NameOrUnknown((MessageCode)err.MessageCode!.Value)}")));
            
            if(success)
                Assert.Pass("Received success");
            else
                Assert.Fail("Received failure");
        }

        protected static SourceContext MakeSourceContext(CompilerOptions compilerOptions = default, string extraSource = default) =>
            SourceContext.CreateAndLoad(new CompilerInput(
                                            new CompilerSource
                                            {
                                                ExtraElementSource = string.IsNullOrEmpty(extraSource)
                                                                         ? Array.Empty<SourceInfo>()
                                                                         : new[] {new SourceInfo("ExtraTestSource", extraSource)}
                                            }, compilerOptions))
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