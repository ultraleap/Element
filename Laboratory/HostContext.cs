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
        public bool IncludePrelude { get; set; }
        public string[] Packages { get; } = Array.Empty<string>();
        public MessageHandler MessageHandler { get; set; } = (messages, anyErrors) =>
        {
            messages.PrintMessagesToTestContext("test output");
            if (anyErrors)
            {
                Assert.Fail("Unexpected error - see below for error received.");
            }
        };
    }
}