using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;
using Expression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public class StructConverter : IBoundaryConverter
    {
        public static StructConverter FromBoundaryStructInfo(IBoundaryStructInfo boundaryStructInfo) => new StructConverter(boundaryStructInfo);

        private readonly IBoundaryStructInfo _boundaryStructInfo;

        private StructConverter(IBoundaryStructInfo boundaryStructInfo) => _boundaryStructInfo = boundaryStructInfo;

        public Result<IValue> LinqToElement(Expression parameter, Context context) =>
            context.EvaluateExpression(_boundaryStructInfo.ElementExpression)
                   .CastInner<Struct>()
                   .Bind(structDeclaration => StructInstance.Create(structDeclaration, _boundaryStructInfo.FieldMap
                                                                                                          .Select(pair => new StructFieldInstruction(parameter, pair.Value))
                                                                                                          .ToArray(), context)
                                                            .Cast<IValue>());

        private class StructFieldInstruction : Instruction, ILinqExpression
        {
            private readonly Expression _parameter;
            private readonly string _clrField;

            public StructFieldInstruction(Expression parameter, string clrField)
            {
                _parameter = parameter;
                _clrField = clrField;
            }

            public Expression Expression => Expression.PropertyOrField(_parameter, _clrField);
            public override IEnumerable<Instruction> Dependent => Array.Empty<Instruction>();
            public override string SummaryString => $"{_parameter}.{_clrField}";
        }

        public Result<Expression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, Context context)
        {
            var obj = Expression.Variable(outputType);
            var assigns = new List<Expression>();
            if (!obj.Type.IsValueType)
            {
                assigns.Add(Expression.Assign(obj, Expression.New(outputType)));
            }

            return value.InstanceType(context)
                        .Check(v => value.Members.Count < 1
                             // TODO: More relevant message code - this error case is always a result of API misuse (using struct converter for non-struct instance)
                             ? context.Trace(EleMessageCode.InvalidBoundaryData, $"Expected instance of a struct with members but got '{value}'")
                             : Result.Success)
                        .Bind(instanceType =>
                         {
                             var builder = new ResultBuilder<Expression>(context, default!);
            
                             foreach (var pair in _boundaryStructInfo.FieldMap)
                             {
                                 var memberExpression = Expression.PropertyOrField(obj, pair.Value);
                                 var fieldResult = value.Index(new Identifier(pair.Key), context).Bind(fieldValue => convertFunction(fieldValue, memberExpression.Type, context));
                                 builder.Append(in fieldResult);
                                 if (!fieldResult.IsSuccess) continue;
                                 assigns.Add(Expression.Assign(memberExpression, fieldResult.ResultOr(default!)));
                             }
            
                             assigns.Add(obj);
                             builder.Result = Expression.Block(new[] {obj}, assigns);
                             return builder.ToResult();
                         });
        }

        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats, Context context) => _boundaryStructInfo.SerializeClrInstance(clrInstance, floats, context);
    }
}