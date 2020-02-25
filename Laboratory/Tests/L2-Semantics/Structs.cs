using Element;
using NUnit.Framework;

namespace Laboratory.Tests
{
    internal class Structs : SemanticsFixture
    {
        public Structs(IHost host) : base(host, "Structs") { }

        [Test]
        public void ConstructInstance() => _host.Evaluate(CompilationInput, "MyStruct(5)");
        
        [Test]
        public void ConstructInstanceWithNestedStruct() => _host.Evaluate(CompilationInput, "NestedStruct(MyStruct(5))");

        [Test]
        public void InstanceMemberAccess() => AssertApproxEqual(CompilationInput, "MyStruct(5).a", "5");

        [Test]
        public void InvalidMemberAccess() => EvaluateExpectingErrorCode(CompilationInput, 7, "MyStruct(10).invalid");

        [Test]
        public void FunctionAsMember() => _host.Evaluate(CompilationInput, "MyStruct(pickSecond)");

        [Test]
        public void ConstrainedMembers() => _host.Evaluate(CompilationInput, "Vector3(5, 10, 15)");

        [
            TestCase("Vector3(1)"),
            TestCase("Vector3(5, 10)"),
            TestCase("Vector3(20, 5, 10, 40)")
        ]
        public void MemberCountNotCorrect(string expression) => EvaluateExpectingErrorCode(CompilationInput, 6, expression);
        
        [Test]
        public void MemberConstraintsNotSatisfied() => EvaluateExpectingErrorCode(CompilationInput, 8, "Vector3(pickSecond, 5, 10)");

        [Test]
        public void AliasAsConstraint() => AssertApproxEqual(CompilationInput, "onlyNumNum(NumNum(9))", "9");

        [Test]
        public void AliasOfAliasAsConstraint() => AssertApproxEqual(CompilationInput, "onlyNumNumNum(NumNumNum(9))", "9");

        [Test]
        public void NoImplicitConversionFromAliasToBase() => EvaluateExpectingErrorCode(CompilationInput, 8, "onlyNum(NumNum(5))");

        [Test]
        public void NoImplicitConversionFromBaseToAlias() => EvaluateExpectingErrorCode(CompilationInput, 8, "onlyNumNum(5)");

        [Test]
        public void IntrinsicInstanceFunction() => AssertApproxEqual(CompilationInput, "6.add(5)", "11");

        [Test]
        public void NonIntrinsicInstanceFunction() => AssertApproxEqual(CompilationInput, "6.addFive", "11");

        [Test]
        public void CustomStructInstanceFunction() => AssertApproxEqual(CompilationInput, "Vector3(5, 5, 5).add(Vector3(10, 10, 10))", "Vector3(15, 15, 15)");

        [Test]
        public void InstanceFunctionRequiresExplicitType() => EvaluateExpectingErrorCode(CompilationInput, 22, "5.addTen");
    }
}