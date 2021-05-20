using Element;
using NUnit.Framework;

namespace Laboratory.Tests.L2.Semantics
{
    internal class _4_CustomStructs : SemanticsFixture
    {
        public _4_CustomStructs() : base("_4_CustomStructs") { }

        [TestCase("MyStruct", "CustomStruct")]
        [TestCase("MyInstance", "MyStruct")]
        public void Typeof(string expression, string type) => AssertTypeof(CompilerInput, expression, type);
        
        [TestCase("MyStruct")]
        [TestCase("Vector3")]
        public void NotDeserializable(string expression) => EvaluateExpectingError(CompilerInput, ElementMessage.SerializationError, expression);

        [Test]
        public void ConstructInstance() => AssertTypeof(CompilerInput, "MyStruct(5)", "MyStruct");
        
        [Test]
        public void ConstructInstanceWithNestedStruct() => AssertTypeof(CompilerInput, "NestedStruct(MyStruct(5))", "NestedStruct");

        [Test]
        public void InstanceMemberAccess() => AssertApproxEqual(CompilerInput, "MyStruct(5).a", "5");

        [Test]
        public void CantIndexStructWithNoAssociatedScope() => EvaluateExpectingError(CompilerInput, ElementMessage.InvalidExpression, "MyStruct(10).invalid");
        
        [Test]
        public void MissingMemberAccess() => EvaluateExpectingError(CompilerInput, ElementMessage.IdentifierNotFound, "Vector3(10, 10, 10).invalid");

        [Test]
        public void FunctionAsMember() => AssertTypeof(CompilerInput, "MyStruct(pickSecond).a", "ExpressionBodiedFunction");
        
        [
            TestCase("Num.acos(0).degrees", "90"),
        ]
        public void ResolvesCorrectLiteralAfterIndexingIntrinsicFunction(string expression, string expected) => AssertApproxEqual(CompilerInput, expression, expected);
        
        [Test]
        public void ConstrainedMembers() => AssertTypeof(CompilerInput, "Vector3(5, 10, 15)", "Vector3");

        [
            TestCase("Vector3(1)"),
            TestCase("Vector3(5, 10)"),
            TestCase("Vector3(20, 5, 10, 40)")
        ]
        public void MemberCountNotCorrect(string expression) => EvaluateExpectingError(CompilerInput, ElementMessage.ArgumentCountMismatch, expression);
        
        [Test]
        public void MemberConstraintsNotSatisfied() => EvaluateExpectingError(CompilerInput, ElementMessage.ConstraintNotSatisfied, "Vector3(pickSecond, 5, 10)");

        [Test]
        public void NoImplicitConversionBetweenStructs() => EvaluateExpectingError(CompilerInput, ElementMessage.ConstraintNotSatisfied, "onlyNum(NumNum(5))");

        [Test]
        public void IntrinsicInstanceFunction() => AssertApproxEqual(CompilerInput, "6.add(5)", "11");

        [Test]
        public void NonIntrinsicInstanceFunction() => AssertApproxEqual(CompilerInput, "6.addFive", "11");

        [Test]
        public void CustomStructInstanceFunction() => AssertApproxEqual(CompilerInput, "Vector3(5, 5, 5).add(Vector3(10, 10, 10))", "Vector3(15, 15, 15)");

        [Test]
        public void InstanceFunctionRequiresExplicitType() => EvaluateExpectingError(CompilerInput, ElementMessage.CannotBeUsedAsInstanceFunction, "5.addTen");
    }
}