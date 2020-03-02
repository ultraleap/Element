using Element;

namespace Laboratory.Tests
{
    internal class HighOrderConstructs : SemanticsFixture
    {
        public HighOrderConstructs(IHost host) : base(host, "HighOrderConstructs") { }

        // Bind a function from indexing
        // Bind a struct instance function
        // Return a function - check using typeof
        // Pass in a function - for, fold?
        // Return a struct
        // Check recursion (using an if to prevent stack overflow)
    }
}