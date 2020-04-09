using NUnit.Framework;

namespace Laboratory.Tests.StandardLibrary
{
    internal class Path : StandardLibraryFixture
    {
        //Paths
        [
            TestCase("Path.Circle(Vector3.Zero, 5).at(0)", "Vector3(0, 5, 0)"),
            TestCase("Path.Circle(Vector3.Zero, 5).at(0.125)", "Vector3(5.mul(1.div(Num.sqrt(2))), 5.mul(1.div(Num.sqrt(2))), 0)"),
            TestCase("Path.Circle(Vector3.Zero, 5).at(0.25)", "Vector3(5, 0, 0)"),
            TestCase("Path.Circle(Vector3.Zero, 5).at(0.375)", "Vector3(5.mul(1.div(Num.sqrt(2))), -5.mul(1.div(Num.sqrt(2))), 0)"),
            TestCase("Path.Circle(Vector3.Zero, 5).at(0.5)", "Vector3(0,-5, 0)"),
            TestCase("Path.Circle(Vector3.Zero, 5).at(0.625)", "Vector3(-5.mul(1.div(Num.sqrt(2))), -5.mul(1.div(Num.sqrt(2))), 0)"),
            TestCase("Path.Circle(Vector3.Zero, 5).at(0.75)", "Vector3(-5, 0, 0)"),
            TestCase("Path.Circle(Vector3.Zero, 5).at(0.875)", "Vector3(-5.mul(1.div(Num.sqrt(2))), 5.mul(1.div(Num.sqrt(2))), 0)"),
            TestCase("Path.Circle(Vector3.Zero, 5).at(1)", "Vector3(0, 5, 0)"),
        ]
        public void EvaluateCirclePath(string expression, string expected) =>
            AssertApproxEqualRelaxed(ValidatedCompilationInput, expected, expression);

        //TEST
        [
            TestCase("Path.Line(Vector3.Zero, Vector3(10, 20, 30)).at(-0.5)", "Vector3(0, 0, 0)"),
            TestCase("Path.Line(Vector3.Zero, Vector3(10, 20, 30)).at(0)", "Vector3(0, 0, 0)"),
            TestCase("Path.Line(Vector3.Zero, Vector3(10, 20, 30)).at(0.5)", "Vector3(5, 7.5, 15)"),
            TestCase("Path.Line(Vector3.Zero, Vector3(10, 20, 30)).at(1)", "Vector3(10, 20, 30)"),
            TestCase("Path.Line(Vector3.Zero, Vector3(10, 20, 30)).at(1.5)", "Vector3(10, 20, 30)"),
        ]
        public void EvaluateLinePath(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
        
        //TEST
        [
            TestCase("Path.RepeatingLine(Vector3.Zero, Vector3(10, 20, 30)).at(-0.5)", "Vector3(5, 7.5, 15)"),
            TestCase("Path.RepeatingLine(Vector3.Zero, Vector3(10, 20, 30)).at(0)", "Vector3(0, 0, 0)"),
            TestCase("Path.RepeatingLine(Vector3.Zero, Vector3(10, 20, 30)).at(0.5)", "Vector3(5, 7.5, 15)"),
            TestCase("Path.RepeatingLine(Vector3.Zero, Vector3(10, 20, 30)).at(1)", "Vector3(10, 20, 30)"),
            TestCase("Path.RepeatingLine(Vector3.Zero, Vector3(10, 20, 30)).at(1.5)", "Vector3(5, 7.5, 15)"),
        ]
        public void EvaluateRepeatingLinePath(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
            
        //TEST
        [
            TestCase("Path.PingPongLine(Vector3.Zero, Vector3(10, 20, 30)).at(-0.5)", "Vector3(-5, -7.5, -15)"),
            TestCase("Path.PingPongLine(Vector3.Zero, Vector3(10, 20, 30)).at(0)", "Vector3(0, 0, 0)"),
            TestCase("Path.PingPongLine(Vector3.Zero, Vector3(10, 20, 30)).at(0.5)", "Vector3(5, 7.5, 15)"),
            TestCase("Path.PingPongLine(Vector3.Zero, Vector3(10, 20, 30)).at(1)", "Vector3(10, 20, 30)"),
            TestCase("Path.PingPongLine(Vector3.Zero, Vector3(10, 20, 30)).at(1.5)", "Vector3(5, 7.5, 15)"),
        ]
        public void EvaluatePingPongLinePath(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
            

        //TEST
        [
            TestCase("Path.Rectangle(10, 10).at(-0.125)", "Vector3(-5, -7.5, 0)"),
            TestCase("Path.Rectangle(10, 10).at(0)", "Vector3(0, 0, 0)"),
            TestCase("Path.Rectangle(10, 10).at(0.125)", "Vector3(0, 0, 0)"),
            TestCase("Path.Rectangle(10, 10).at(0.25)", "Vector3(0, 0, 0)"),
            TestCase("Path.Rectangle(10, 10).at(0.375)", "Vector3(0, 0, 0)"),
            TestCase("Path.Rectangle(10, 10).at(0.5)", "Vector3(5, 7.5, 0)"),
            TestCase("Path.Rectangle(10, 10).at(0.625)", "Vector3(0, 0, 0)"),
            TestCase("Path.Rectangle(10, 10).at(0.75)", "Vector3(0, 0, 0)"),
            TestCase("Path.Rectangle(10, 10).at(0.875)", "Vector3(0, 0, 0)"),
            TestCase("Path.Rectangle(10, 10).at(1)", "Vector3(0, 0, 0)"),
            TestCase("Path.Rectangle(10, 10).at(1.125)", "Vector3(5, 7.5, 0)"),
        ]
        public void EvaluateRectanglePath(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
       

        //TODO
        public void EvaluateLissajousPath(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression); 


        //TODO
        public void EvaluateRosePath(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression); 
    
    
        //Modifiers
        
        
        //TODO
        public void EvaluateWithBounds(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression); 
        //TODO
        public void EvaluateLineBetween(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression); 
        
        //TODO
        public void EvaluateConcatenate(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
        
        //TODO
        public void EvaluateAnimate(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);  
        
        //TODO
        public void EvaluateOffset(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression); 
        
        //TODO
        public void EvaluateReverse(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);  
        
        //TODO
        public void EvaluateTranslate(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);  
        
        //TODO
        public void EvaluateTransform(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
    }
    
      internal class BoundedPath : StandardLibraryFixture
    {
        //TODO
        public void EvaluatePingPong(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
        
        //TODO
        public void EvaluateRepeat(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
        
        //TODO
        public void EvaluateEasing(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);
        
        //TODO
        public void EvaluatePathSegment(string expression, string expected) =>
            AssertApproxEqual(ValidatedCompilationInput, expected, expression);

    }
      
}