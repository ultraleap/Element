using System;
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;

namespace Element.NET.Tests
{
    public abstract class FixtureBase
    {
        protected static void DoExpectingMessageCode(int messageCode, Func<SourceContext, bool> funcReturningSuccessStatus, string extraSource = default)
        {
            var errors = new List<CompilerMessage>();
            var src = MakeSourceContext(logCallback: ExpectMessageCode(messageCode, errors), extraSource: extraSource);
            if (funcReturningSuccessStatus(src))
            {
                if (errors.Count > 0) Assert.Fail("Expected message code '{0}' but got following code(s): {1}", messageCode, string.Join(",", errors.Select(err => err.MessageCode)));
                else Assert.Fail("Expected message code '{0}' but evaluation succeeded", messageCode);
            }
            
            Assert.Fail("Expected message code '{0}' but no errors were logged", messageCode);
        }
        
        protected static void FailOnError(CompilerMessage message)
        {
            if (message.MessageLevel >= MessageLevel.Error) Assert.Fail(message.ToString());
            else TestContext.WriteLine(message.ToString());
        }
        
        protected static Action<CompilerMessage> ExpectMessageCode(int messageCode, List<CompilerMessage> errorsReceived) => message =>
        {
            if (message.MessageCode == messageCode) Assert.Pass($"Received expected message code {messageCode}");
            if (message.MessageLevel >= MessageLevel.Error) errorsReceived.Add(message);
            else TestContext.WriteLine(message.ToString());
        };

        protected static SourceContext MakeSourceContext(CompilationInput compilationInput = default, Action<CompilerMessage> logCallback = default, string extraSource = default)
        {
            var result = SourceContext.TryCreate(compilationInput ?? new CompilationInput(logCallback ?? FailOnError), out var sourceContext)
                       ? sourceContext
                       : null;
            if (!string.IsNullOrEmpty(extraSource))
            {
                result?.LoadElementSourceString(new SourceInfo("ExtraTestSource", extraSource));
            }

            if (result == null)
            {
                Assert.Fail("Failed to create source context");
            }

            return result!; // Assert when null stops us getting here
        }
    }
}