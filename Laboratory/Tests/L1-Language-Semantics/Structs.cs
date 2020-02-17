using System.IO;
using Element;

namespace Laboratory.Tests
{
    internal class Structs : HostFixture
    {
        public Structs(IHost host) : base(host) { }
        
        private static FileInfo[] SourceFiles => new []{GetTestFile("Structs")};

        private static CompilationInput CompilationInput => new CompilationInput(FailOnError, true, extraSourceFiles: SourceFiles);
        
        // Any accepts num
        // Num doesn't accept non-num
        
        // Create instance of custom struct
            // Constructor call - check type of return
        // Index instance of custom struct
            // Can access members of struct instance e.g. Vector2(5, 10).x returns 5
            // Invalid member returns error
        // Create struct which includes other struct as instance
            // 
        // Can put a function in struct (using any type for now)
        // Can constrain function inputs/return to custom struct
            // Doesn't accept wrong type
            // Accepts instance of type
    }
}