using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class _4_CustomStructs : SemanticsFixture
    {
        public _4_CustomStructs() : base("_4_CustomStructs") { }

        [TestCase("MyStruct", "CustomStruct")]
        [TestCase("MyInstance", "MyStruct")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilationInput, expression, type);
        
        [TestCase("MyStruct")]
        [TestCase("Vector3")]
        public void NotDeserializable(string expression) => EvaluateExpectingErrorCode(CompilationInput, MessageCode.SerializationError, expression);

        [Test]
        public void ConstructInstance() => AssertTypeof(CompilationInput, "MyStruct(5)", "MyStruct");
        
        [Test]
        public void ConstructInstanceWithNestedStruct() => AssertTypeof(CompilationInput, "NestedStruct(MyStruct(5))", "NestedStruct");

        [Test]
        public void InstanceMemberAccess() => AssertApproxEqual(CompilationInput, "MyStruct(5).a", "5");

        [Test]
        public void CantIndexStructWithNoAssociatedScope() => EvaluateExpectingErrorCode(CompilationInput, MessageCode.InvalidExpression, "MyStruct(10).invalid");
        
        [Test]
        public void MissingMemberAccess() => EvaluateExpectingErrorCode(CompilationInput, MessageCode.IdentifierNotFound, "Vector3(10, 10, 10).invalid");

        [Test]
        public void FunctionAsMember() => AssertTypeof(CompilationInput, "MyStruct(pickSecond).a", "ExpressionBodiedFunction");
        
        [
            TestCase("Num.cos(0).degrees", "90"),
        ]
        public void ResolvesCorrectLiteralAfterIndexingIntrinsicFunction(string expression, string expected) => AssertApproxEqual(CompilationInput, expression, expected);
        
        [Test]
        public void ConstrainedMembers() => AssertTypeof(CompilationInput, "Vector3(5, 10, 15)", "Instance:Vector3:Struct");

        [
            TestCase("Vector3(1)"),
            TestCase("Vector3(5, 10)"),
            TestCase("Vector3(20, 5, 10, 40)")
        ]
        public void MemberCountNotCorrect(string expression) => EvaluateExpectingErrorCode(CompilationInput, MessageCode.ArgumentCountMismatch, expression);
        
        [Test]
        public void MemberConstraintsNotSatisfied() => EvaluateExpectingErrorCode(CompilationInput, MessageCode.ConstraintNotSatisfied, "Vector3(pickSecond, 5, 10)");

        [Test]
        public void NoImplicitConversionBetweenStructs() => EvaluateExpectingErrorCode(CompilationInput, MessageCode.ConstraintNotSatisfied, "onlyNum(NumNum(5))");

        [Test]
        public void IntrinsicInstanceFunction() => AssertApproxEqual(CompilationInput, "6.add(5)", "11");

        [Test]
        public void NonIntrinsicInstanceFunction() => AssertApproxEqual(CompilationInput, "6.addFive", "11");

        [Test]
        public void CustomStructInstanceFunction() => AssertApproxEqual(CompilationInput, "Vector3(5, 5, 5).add(Vector3(10, 10, 10))", "Vector3(15, 15, 15)");

        [Test]
        public void InstanceFunctionRequiresExplicitType() => EvaluateExpectingErrorCode(CompilationInput, MessageCode.CannotBeUsedAsInstanceFunction, "5.addTen");
    }
}