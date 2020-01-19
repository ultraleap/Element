using System;
using System.Collections.Generic;
using NUnit.Framework;

namespace Laboratory
{
    internal delegate void MessageHandler(List<string> messages, bool anyErrors);

    /// <summary>
    /// Contain data for configuring host behaviour
    /// </summary>
    internal class HostContext
    {
        public static readonly MessageHandler PrintMessagesAndAssertNoErrors = (messages, anyErrors) =>
        {
            messages.PrintMessagesToTestContext("test output");
            if (anyErrors)
            {
                Assert.Fail("Unexpected error - see below for error received.");
            }
        };
        
        public static readonly MessageHandler PrintMessages = (messages, anyErrors) =>
        {
            messages.PrintMessagesToTestContext("test output");
        };
        
        public bool IncludePrelude { get; set; } = false;
        public string[] Packages { get; } = Array.Empty<string>();
        public MessageHandler MessageHandler { get; set; } = PrintMessagesAndAssertNoErrors;
    }
}