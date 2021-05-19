using System;
using System.Collections;
using NUnit.Framework;
using ResultNET;

namespace Element.NET.TestHelpers
{
    public abstract class HostFixture : FixtureBase
    {
        protected readonly IHost Host = HostArguments.MakeHost();
        
        public enum EvaluationMode
        {
            Compiled,
            Interpreted
        }
        
        public static float ToRadians(float val) => (float)Math.PI / 180f * val;

        protected abstract class EvaluationBase
        {
            public abstract Result<float[]> Evaluate(CompilerInput input, IHost host);
        }

        protected class ExpressionEvaluation : EvaluationBase
        {
            public ExpressionEvaluation(string expression, bool interpreted)
            {
                Expression = expression;
                Interpreted = interpreted;
            }
            
            public ExpressionEvaluation(string expression, EvaluationMode mode)
            {
                Expression = expression;
                Interpreted = mode == EvaluationMode.Interpreted;
            }
            
            public string Expression { get; }
            public bool Interpreted { get; }
            public override Result<float[]> Evaluate(CompilerInput input, IHost host) => host.EvaluateExpression(input, Expression, Interpreted);
        }
        
        protected class FunctionEvaluation : EvaluationBase
        {
            public FunctionEvaluation(string functionExpression, string callExpression, bool interpreted)
            {
                FunctionExpression = functionExpression;
                CallExpression = callExpression;
                Interpreted = interpreted;
            }
            
            public FunctionEvaluation(string functionExpression, string callExpression, EvaluationMode mode)
            {
                FunctionExpression = functionExpression;
                CallExpression = callExpression;
                Interpreted = mode == EvaluationMode.Interpreted;
            }

            public string FunctionExpression { get; }
            public string CallExpression { get; }
            public bool Interpreted { get; }
            public override Result<float[]> Evaluate(CompilerInput input, IHost host) => host.EvaluateFunction(input, FunctionExpression, CallExpression, Interpreted);
        }

        private void AssertApproxEqual(Result<float[]> result, Result<float[]> expected, IComparer comparer) =>
            result
                .Accumulate(() => expected)
                .Switch((t, messages) =>
                        {
                            var (result, expected) = t;
                            LogMessages(messages);
                            CollectionAssert.AreEqual(expected, result, comparer);
                        },
                       messages => ExpectingSuccess(messages, false));

        protected void AssertApproxEqual(CompilerInput compilerInput, EvaluationBase controlEvaluation, float[] results) =>
            AssertApproxEqual(controlEvaluation.Evaluate(compilerInput, Host), results, FloatComparer);

        // TODO: Remove this in favour using ExpressionEvaluation
        protected void AssertApproxEqual(CompilerInput compilerInput, string controlEvaluation, string expectedExpression) =>
            AssertApproxEqual(compilerInput, new ExpressionEvaluation(controlEvaluation, true), new ExpressionEvaluation(expectedExpression, true));

        protected void AssertApproxEqual(CompilerInput compilerInput, EvaluationBase controlEvaluation, EvaluationBase expected) =>
            AssertApproxEqual(controlEvaluation.Evaluate(compilerInput, Host),
                              expected.Evaluate(compilerInput, Host),
                              FloatComparer);

        protected void AssertApproxEqual(CompilerInput compilerInput, (string ConstantExpression, string ExpectedExpression) args, EvaluationMode evaluationMode) =>
            AssertApproxEqual(compilerInput,
                              new ExpressionEvaluation(args.ConstantExpression, evaluationMode == EvaluationMode.Interpreted),
                              new ExpressionEvaluation(args.ExpectedExpression, true));

        protected void AssertApproxEqual(CompilerInput compilerInput, (string FunctionExpression, string CallExpression, string ExpectedExpression) args, EvaluationMode evaluationMode) =>
            AssertApproxEqual(compilerInput,
                              new FunctionEvaluation(args.FunctionExpression, args.CallExpression, evaluationMode == EvaluationMode.Interpreted),
                              new ExpressionEvaluation(args.ExpectedExpression, true));

        protected void AssertApproxEqualRelaxed(CompilerInput compilerInput, EvaluationBase controlEvaluation, EvaluationBase expected) =>
            AssertApproxEqual(controlEvaluation.Evaluate(compilerInput, Host),
                              expected.Evaluate(compilerInput, Host),
                              RelaxedFloatComparer);

        protected void AssertApproxEqualRelaxed(CompilerInput compilerInput, (string FunctionExpression, string CallExpression, string ExpectedExpression) args, EvaluationMode evaluationMode) =>
            AssertApproxEqualRelaxed(compilerInput,
                                     new FunctionEvaluation(args.FunctionExpression, args.CallExpression, evaluationMode == EvaluationMode.Interpreted),
                                     new ExpressionEvaluation(args.ExpectedExpression, true));
        
        // TODO: Remove this in favour using ExpressionEvaluation
        protected void AssertTypeof(CompilerInput compilerInput, string expression, string typeStr) =>
            Host.Typeof(compilerInput, expression)
                .Switch((result, messages) =>
                {
                    LogMessages(messages);
                    Assert.That(result, Is.EqualTo(typeStr));
                }, messages => ExpectingSuccess(messages, false));
        
        protected void AssertTypeof(CompilerInput compilerInput, ExpressionEvaluation expression, string typeStr) =>
            Host.Typeof(compilerInput, expression.Expression)
                .Switch((result, messages) =>
                {
                    LogMessages(messages);
                    Assert.That(result, Is.EqualTo(typeStr));
                }, messages => ExpectingSuccess(messages, false));
        
        // TODO: Remove this in favour using ExpressionEvaluation
        protected void EvaluateExpectingError(CompilerInput compilerInput, MessageInfo info, string expression)
        {
            var result = new ExpressionEvaluation(expression, true).Evaluate(compilerInput, Host);
            ExpectingError(result.Messages, result.IsSuccess, info);
        }
        
        protected void EvaluateExpectingError(CompilerInput compilerInput, MessageInfo info, EvaluationBase evaluation)
        {
            var result = evaluation.Evaluate(compilerInput, Host);
            ExpectingError(result.Messages, result.IsSuccess, info);
        }
    }
}