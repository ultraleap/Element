using NUnit.Framework;

namespace Laboratory.Tests.L3.Prelude
{
	internal class Boolean : PreludeFixture
	{
		/*[Theory]
		public void Constants((string ConstantExpression,string ExpectedExpression) args, EvaluationMode evaluationMode) =>
			AssertApproxEqual(ValidatedCompilerInput, args, evaluationMode);

		[DatapointSource] public (string ConstantExpression, string ExpectedExpression)[] ConstantExpressionData =
		{

		};*/
		
		[Theory]
		public void FunctionCalls((string FunctionExpression, string CallExpression, string ExpectedExpression) args, EvaluationMode evaluationMode) =>
			AssertApproxEqual(ValidatedCompilerInput, args, evaluationMode);

		[DatapointSource] public (string FunctionExpression, string CallExpression, string ExpectedExpression)[] FunctionCallData =
		{
			("Bool" ,"(0)", "False"),
			("Bool" ,"(1)", "True"),
			("Bool" ,"(1.2)", "True"),
			("Bool" ,"(9999)", "True"),
			("Bool" ,"(0.1)", "True"),
			("Bool" ,"(-0.1)", "False"),
			
			("Bool.not" ,"(True)", "False"),
			("Bool.not" ,"(False)", "True"),
			
			("False.and" ,"(False)", "False"),
			("False.and" ,"(True)", "False"),
			("True.and" ,"(False)", "False"),
			("True.and" ,"(True)", "True"),
			
			("False.or" ,"(False)", "False"),
			("False.or" ,"(True)", "True"),
			("True.or" ,"(False)", "True"),
			("True.or" ,"(True)", "True"),
			
			("False.xor" ,"(False)", "False"),
			("False.xor" ,"(True)", "True"),
			("True.xor" ,"(False)", "True"),
			("True.xor" ,"(True)", "False"),
			
			("0.lt" ,"(0)", "False"),
			("0.2.lt" ,"(0)", "False"),
			("-0.2.lt" ,"(0)", "True"),
			("1.lt" ,"(2)", "True"),
			
			("0.gt" ,"(0)", "False"),
			("0.2.gt" ,"(0)", "True"),
			("-0.2.gt" ,"(0)", "False"),
			("2.gt" ,"(1)", "True"),
			
			("0.leq" ,"(0)", "True"),
			("0.2.leq" ,"(0)", "False"),
			("-0.2.leq" ,"(0)", "True"),
			("1.leq" ,"(2)", "True"),
			
			("0.geq" ,"(0)", "True"),
			("0.2.geq" ,"(0)", "True"),
			("-0.2.geq" ,"(0)", "False"),
			("2.geq" ,"(1)", "True"),
			
			("0.eq" ,"(0)", "True"),
			("1.eq" ,"(0)", "False"),
			("0.1.eq" ,"(0)", "False"),
			("999.999.eq" ,"(999.999)", "True"),
			
			("0.neq" ,"(0)", "False"),
			("1.neq" ,"(0)", "True"),
			("0.1.neq" ,"(0)", "True"),
			("999.999.neq" ,"(999.999)", "False"),
			
			("_(cond:Bool, a:Num, b:Num):Num = Bool.if(cond, a, b)" ,"(True, 1, 0)", "1"),
			("_(cond:Bool, a:Num, b:Num):Num = Bool.if(cond, a, b)" ,"(False, 1, 0)", "0"),
		};
	}
}