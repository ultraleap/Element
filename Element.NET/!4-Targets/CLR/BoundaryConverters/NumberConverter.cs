using System;
using System.Collections.Generic;
using Element.AST;
using ResultNET;
using Expression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public class NumberConverter : IBoundaryConverter
    {
        private NumberConverter(){}
        public static NumberConverter Instance { get; } = new NumberConverter();

        public Result<IValue> LinqToElement(Expression parameter, Context context) =>
            parameter.Type switch
            {
                {} t when t == typeof(bool) => new NumberInstruction(Expression.Condition(parameter, Expression.Constant(1f), Expression.Constant(0f)), BoolStruct.Instance),
                _                           => new NumberInstruction(Expression.Convert(parameter, typeof(float)))
            };

        public Result<Expression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, Context context) =>
            convertFunction(value, typeof(float), context)
               .Map(convertedValue =>
                    (outputType, convertedValue) switch
                    {
                        (_, {} result) when result.Type == outputType => result, // Correct type from convert, return directly
                        ({} t, {} result) when t == typeof(bool)      => Expression.GreaterThan(result, Expression.Constant(0f)),
                        (_, {} result)                                => Expression.Convert(result, outputType),
                        _                                             => throw new InternalCompilerException($"Unhandled {nameof(ElementToLinq)} output type")
                    });

        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats, Context context) =>
            (clrInstance switch
            {
                float f => new Result<float>(f),
                bool b  => new Result<float>(b ? 1f : 0f),
                _       => context.Trace(ElementMessage.SerializationError, $"{nameof(NumberConverter)} doesn't support serializing '{clrInstance}'")
            })
           .Bind(f =>
            {
                floats.Add(f);
                return Result.Success;
            });

        private class NumberInstruction : Instruction, ILinqExpression
        {
            public NumberInstruction(Expression parameter, IIntrinsicStructImplementation? typeOverride = default)
                : base(typeOverride) =>
                Parameter = parameter;

            public Expression Parameter { get; }
            public override IEnumerable<Instruction> Dependent => Array.Empty<Instruction>();
            public Expression Expression => Parameter;
            public override string SummaryString => Parameter.ToString();
            // ReSharper disable once PossibleUnintendedReferenceComparison
            public override bool Equals(Instruction other) => other == this;
            public override int GetHashCode() => new {Parameter, InstanceTypeOverride = StructImplementation}.GetHashCode();
        }
    }
}