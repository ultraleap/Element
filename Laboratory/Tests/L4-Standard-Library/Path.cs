using NUnit.Framework;

namespace Laboratory.Tests.L4.StandardLibrary
{
    internal class Paths : StandardLibraryFixture
    {
        /*TestCase("Path.circle(Vector3.zero, 5).at(0)", "Vector3(0, 5, 0)"),
        TestCase("Path.circle(Vector3.zero, 5).at(0.125)", "Vector3(5.mul(1.div(Num.sqrt(2))), 5.mul(1.div(Num.sqrt(2))), 0)"),
        TestCase("Path.circle(Vector3.zero, 5).at(0.25)", "Vector3(5, 0, 0)"),
        TestCase("Path.circle(Vector3.zero, 5).at(0.375)", "Vector3(5.mul(1.div(Num.sqrt(2))), -5.mul(1.div(Num.sqrt(2))), 0)"),
        TestCase("Path.circle(Vector3.zero, 5).at(0.5)", "Vector3(0,-5, 0)"),
        TestCase("Path.circle(Vector3.zero, 5).at(0.625)", "Vector3(-5.mul(1.div(Num.sqrt(2))), -5.mul(1.div(Num.sqrt(2))), 0)"),
        TestCase("Path.circle(Vector3.zero, 5).at(0.75)", "Vector3(-5, 0, 0)"),
        TestCase("Path.circle(Vector3.zero, 5).at(0.875)", "Vector3(-5.mul(1.div(Num.sqrt(2))), 5.mul(1.div(Num.sqrt(2))), 0)"),
        TestCase("Path.circle(Vector3.zero, 5).at(1)", "Vector3(0, 5, 0)"),
        TestCase("Path.repeatingLine(Vector3.zero, Vector3(10, 20, 30)).at(-0.25)", "Vector3(7.5, 15, 22.5)"),
        TestCase("Path.repeatingLine(Vector3.zero, Vector3(10, 20, 30)).at(0)", "Vector3(0, 0, 0)"),
        TestCase("Path.repeatingLine(Vector3.zero, Vector3(10, 20, 30)).at(0.25)", "Vector3(2.5, 5, 7.5)"),
        TestCase("Path.repeatingLine(Vector3.zero, Vector3(10, 20, 30)).at(0.5)", "Vector3(5, 10, 15)"),
        TestCase("Path.repeatingLine(Vector3.zero, Vector3(10, 20, 30)).at(0.75)", "Vector3(7.5, 15, 22.5)"),
        TestCase("Path.repeatingLine(Vector3.zero, Vector3(10, 20, 30)).at(1)", "Vector3(0, 0, 0)"),
        TestCase("Path.repeatingLine(Vector3.zero, Vector3(10, 20, 30)).at(1.25)", "Vector3(2.5, 5, 7.5)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(-2)", "Vector3(0, 0, 0)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(-1.75)", "Vector3(2.5, 5, 7.5)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(-1.5)", "Vector3(5, 10, 15)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(-1.25)", "Vector3(7.5, 15, 22.5)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(-1)", "Vector3(10, 20, 30)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(-0.75)", "Vector3(7.5, 15, 22.5)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(-0.5)", "Vector3(5, 10, 15)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(-0.25)", "Vector3(2.5, 5, 7.5)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(0)", "Vector3(0, 0, 0)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(0.25)", "Vector3(2.5, 5, 7.5)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(0.5)", "Vector3(5, 10, 15)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(0.75)", "Vector3(7.5, 15, 22.5)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(1)", "Vector3(10, 20, 30)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(1.25)", "Vector3(7.5, 15, 22.5)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(1.5)", "Vector3(5, 10, 15)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(1.75)", "Vector3(2.5, 5, 7.5)"),
        TestCase("Path.pingPongLine(Vector3.zero, Vector3(10, 20, 30)).at(2)", "Vector3(0, 0, 0)"),
        TestCase("Path.rectangle(10, 10).at(-1.125)", "Vector3(-5, 0, 0)"),
        TestCase("Path.rectangle(10, 10).at(-1)", "Vector3(-5, -5, 0)"),
        TestCase("Path.rectangle(10, 10).at(-0.875)", "Vector3(0, -5, 0)"),
        TestCase("Path.rectangle(10, 10).at(-0.75)", "Vector3(5, -5, 0)"),
        TestCase("Path.rectangle(10, 10).at(-0.625)", "Vector3(5, 0, 0)"),
        TestCase("Path.rectangle(10, 10).at(-0.45)", "Vector3(3, 5, 0)"),
        TestCase("Path.rectangle(10, 10).at(-0.5)", "Vector3(5, 5, 0)"),
        TestCase("Path.rectangle(10, 10).at(-0.55)", "Vector3(5, 3, 0)"),
        TestCase("Path.rectangle(10, 10).at(-0.375)", "Vector3(0, 5, 0)"),
        TestCase("Path.rectangle(10, 10).at(-0.25)", "Vector3(-5, 5, 0)"),
        TestCase("Path.rectangle(10, 10).at(-0.125)", "Vector3(-5, 0, 0)"),
        TestCase("Path.rectangle(10, 10).at(0)", "Vector3(-5, -5, 0)"),
        TestCase("Path.rectangle(10, 10).at(0.125)", "Vector3(0, -5, 0)"),
        TestCase("Path.rectangle(10, 10).at(0.25)", "Vector3(5, -5, 0)"),
        TestCase("Path.rectangle(10, 10).at(0.375)", "Vector3(5, 0, 0)"),
        TestCase("Path.rectangle(10, 10).at(0.5)", "Vector3(5, 5, 0)"),
        TestCase("Path.rectangle(10, 10).at(0.625)", "Vector3(0, 5, 0)"),
        TestCase("Path.rectangle(10, 10).at(0.75)", "Vector3(-5, 5, 0)"),
        TestCase("Path.rectangle(10, 10).at(0.875)", "Vector3(-5, 0, 0)"),
        TestCase("Path.rectangle(10, 10).at(1)", "Vector3(-5, -5, 0)"),
        TestCase("Path.rectangle(10, 10).at(1.125)", "Vector3(0, -5, 0)"),
                
        //TODO EvaluateLissajousPath 
        //TODO EvaluateRosePath 
        
        //Modifiers
        
        //TODO EvaluateWithBounds 
        //TODO EvaluateLineBetween 
        //TODO EvaluateConcatenate
        //TODO EvaluateAnimate  
        //TODO EvaluateOffset 
        //TODO EvaluateReverse  
        //TODO EvaluateTranslate  
        //TODO EvaluateTransform
        
        // Bounded Paths
        
        TestCase("BoundedPath.line(Vector3.zero, Vector3(10, 20, 30)).atBounded(-0.5)", "Vector3(0, 0, 0)"),
        TestCase("BoundedPath.line(Vector3.zero, Vector3(10, 20, 30)).atBounded(0)", "Vector3(0, 0, 0)"),
        TestCase("BoundedPath.line(Vector3.zero, Vector3(10, 20, 30)).atBounded(0.5)", "Vector3(5, 10, 15)"),
        TestCase("BoundedPath.line(Vector3.zero, Vector3(10, 20, 30)).atBounded(1)", "Vector3(10, 20, 30)"),
        TestCase("BoundedPath.line(Vector3.zero, Vector3(10, 20, 30)).atBounded(1.5)", "Vector3(10, 20, 30)"),*/
            
        //Modifiers
        
        //TODO EvaluatePingPong
        //TODO EvaluateRepeat
        //TODO EvaluateEasing
        //TODO EvaluatePathSegment
    }
      
}