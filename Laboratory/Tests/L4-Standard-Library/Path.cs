using NUnit.Framework;

namespace Laboratory.Tests.StandardLibrary
{
    internal class Path : StandardLibraryFixture
    {
        [
            TestCase("Path.Circle(Vector3.Zero, 5).at(0)", "Vector3(0, 5, 0)"),
            TestCase("Path.Circle(Vector3.Zero, 5).at(0.5)", "Vector3(-4.371139e-07,-5, 0)"),
        ]
        public void Circle(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
        
        // [
        //     TestCase("Path.Circle(Vector3.Zero, 5).at(0)", "Vector3(0, 5, 0)"),
        //     TestCase("Path.Circle(Vector3.Zero, 5).at(0.5)", "Vector3(-4.371139e-07,-5, 0)"),
        // ]
        // public void Circle(string expression, string expected) =>
        //     AssertApproxEqual(ValidatedCompilationInput, expected, expression);
    }
}