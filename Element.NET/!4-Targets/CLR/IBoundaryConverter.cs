using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using Element.AST;
using LExpression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public delegate Result<System.Linq.Expressions.Expression> ConvertFunction(IValue value, Type outputType, ITrace trace);
    
    public interface IBoundaryConverter
    {
        Result<IValue> LinqToElement(LExpression parameter, IBoundaryConverter root, CompilationContext context);
        Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, CompilationContext context);
    }

    public class NumberConverter : IBoundaryConverter
    {
        private NumberConverter(){}
        public static NumberConverter Instance { get; } = new NumberConverter();

        public Result<IValue> LinqToElement(System.Linq.Expressions.Expression parameter, IBoundaryConverter root, CompilationContext context) =>
            parameter.Type switch
        {
            {} t when t == typeof(bool) => new NumberExpression(LExpression.Condition(parameter, LExpression.Constant(1f), LExpression.Constant(0f)), BoolStruct.Instance),
            _ => new NumberExpression(LExpression.Convert(parameter, typeof(float)))
        };

        public Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, CompilationContext context) =>
            convertFunction(value, typeof(float), context)
                .Map(convertedValue =>
                          (outputType, convertedValue) switch
                          {
                              (_, {} result) when result.Type == outputType => result, // Correct type from convert, return directly
                              ({} t, {} result) when t == typeof(bool) => LExpression.LessThanOrEqual(result, LExpression.Constant(0f)),
                              (_, {} result) => LExpression.Convert(result, outputType),
                              _ => throw new InternalCompilerException($"Unhandled {nameof(ElementToLinq)} output type")
                          });

        private class NumberExpression : Expression, ICLRExpression
        {
            public NumberExpression(LExpression parameter, IntrinsicStructImplementation? typeOverride = default)
                : base(typeOverride) =>
                Parameter = parameter;

            public LExpression Parameter { get; }
            public override IEnumerable<Expression> Dependent => Array.Empty<Expression>();
            public LExpression Compile(Func<Expression, LExpression> compileOther) => Parameter;
            protected override string ToStringInternal() => Parameter.ToString();
            public override bool Equals(Expression other) => other == this;
            public override int GetHashCode() => new {Parameter, InstanceTypeOverride = StructImplementation}.GetHashCode();
        }
    }

    [AttributeUsage(AttributeTargets.Struct)]
    public class ElementStructTemplateAttribute : Attribute
    {
        public string ElementTypeExpression { get; }
        public ElementStructTemplateAttribute(string elementTypeExpression)
        {
            ElementTypeExpression = elementTypeExpression;
        }
    }

    public class StructConverter : IBoundaryConverter
    {
        private readonly string _elementTypeExpression;
        private readonly Dictionary<string, string> _elementToClrFieldMapping;

        public StructConverter(string elementTypeExpression, Dictionary<string, string> fieldMapping)
        {
            _elementTypeExpression = elementTypeExpression;
            _elementToClrFieldMapping = fieldMapping;
        }

        public StructConverter(string elementTypeExpression, Type structType)
        {
            _elementTypeExpression = elementTypeExpression;
            _elementToClrFieldMapping = structType.GetFields().ToDictionary(f => f.Name, f => $"{char.ToLower(f.Name[0])}{f.Name.Substring(1)}");
        }

        public Result<IValue> LinqToElement(LExpression parameter, IBoundaryConverter root, CompilationContext context) =>
            context.SourceContext.EvaluateExpression(_elementTypeExpression)
                   .Cast<Struct>(context)
                   .Map(structDeclaration => (IValue)new StructInstance(structDeclaration, _elementToClrFieldMapping.Select(pair => new FieldExpression(root, parameter, pair.Value))));

        private class FieldExpression : Expression, ICLRExpression
        {
            private readonly LExpression _parameter;
            //private readonly IBoundaryConverter _root;
            private readonly string _clrField;

            public FieldExpression(IBoundaryConverter root, LExpression parameter, string clrField)
            {
                //_root = root;
                _parameter = parameter;
                _clrField = clrField;
                Dependent = Array.Empty<Expression>();
            }

            public LExpression Compile(Func<Expression, LExpression> compileOther) =>
                LExpression.PropertyOrField(_parameter, _clrField);
            public override IEnumerable<Expression> Dependent { get; }
            protected override string ToStringInternal() => $"{_parameter}.{_clrField}";
            /*public Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
                _root.LinqToElement(LExpression.PropertyOrField(_parameter, _clrField), _root, context);*/
        }

        public Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, CompilationContext context)
        {
            var obj = LExpression.Variable(outputType);
            var assigns = new List<LExpression>();
            if (!obj.Type.IsValueType)
            {
                assigns.Add(LExpression.Assign(obj, LExpression.New(outputType)));
            }
            
            if (!(value is StructInstance structInstance)) return context.Trace(MessageCode.InvalidBoundaryData, $"'{value}' is not a struct instance - expected struct instance when converting a struct");
            var builder = new ResultBuilder<LExpression>(context, default!);
            
            foreach (var pair in _elementToClrFieldMapping)
            {
                var memberExpression = LExpression.PropertyOrField(obj, pair.Value);
                var fieldResult = structInstance.Index(new Identifier(pair.Key), context).Bind(fieldValue => convertFunction(fieldValue, memberExpression.Type, context));
                builder.Append(in fieldResult);
                if (!fieldResult.IsSuccess) continue;
                assigns.Add(LExpression.Assign(memberExpression, fieldResult.ResultOr(default!)));
            }
            
            assigns.Add(obj);
            builder.Result = LExpression.Block(new[] {obj}, assigns);
            return builder.ToResult();
        }
    }
    
    
    
    public class BoundaryConverter : Dictionary<Type, IBoundaryConverter>, IBoundaryConverter
    {
        public BoundaryConverter() : this(CreateDefault()) { }
        public BoundaryConverter(IDictionary<Type, IBoundaryConverter> mappings) : base(mappings) { }
        public static Dictionary<Type, IBoundaryConverter> CreateDefault()
        {
            //var rect = new Input1D(new Map {{"x", "X"}, {"y", "Y"}, {"width", "Width"}, {"height", "Height"}});
            return new Dictionary<Type, IBoundaryConverter>
            {
                {typeof(float), NumberConverter.Instance},
                {typeof(int), NumberConverter.Instance},
                {typeof(double), NumberConverter.Instance},
                {typeof(long), NumberConverter.Instance},
                {typeof(bool), NumberConverter.Instance},
                {typeof(Vector2), new StructConverter("Vector2", new Dictionary<string, string>{{"x", "X"}, {"y", "Y"}})},
                {typeof(Vector3), new StructConverter("Vector3", new Dictionary<string, string>{{"x", "X"}, {"y", "Y"}, {"z", "Z"}})},
                
                /*{typeof(Point), vector2},
                {typeof(PointF), vector2},
                {typeof(Vector3), new Input1D(new Map {{"x", "X"}, {"y", "Y"}, {"z", "Z"}})},
                {typeof(Vector4), new Input1D(new Map {{"x", "X"}, {"y", "Y"}, {"z", "Z"}, {"w", "W"}})},
                {typeof(Rectangle), rect},
                {typeof(RectangleF), rect},
                {
                    typeof(Matrix4x4), new Input2D(new Dictionary<string, object>
                    {
                        {"x", new Map {{"x", "M11"}, {"y", "M12"}, {"z", "M13"}, {"w", "M14"}}},
                        {"y", new Map {{"x", "M21"}, {"y", "M22"}, {"z", "M23"}, {"w", "M24"}}},
                        {"z", new Map {{"x", "M31"}, {"y", "M32"}, {"z", "M33"}, {"w", "M34"}}},
                        {"w", new Map {{"x", "M41"}, {"y", "M42"}, {"z", "M43"}, {"w", "M44"}}}
                    })
                },
                {
                    typeof(Matrix3x2), new Input2D(new Dictionary<string, object>
                    {
                        {"x", new Map {{"x", "M11"}, {"y", "M12"}}},
                        {"y", new Map {{"x", "M21"}, {"y", "M22"}}},
                        {"z", new Map {{"x", "M31"}, {"y", "M32"}}}
                    })
                },
                {typeof(float[]), new Array1D()}*/
            };
        }

        private Result<IBoundaryConverter> TryAddStructConverter(Type clrStructType, ITrace trace)
        {
            if (!(clrStructType.GetCustomAttribute<ElementStructTemplateAttribute>() is {} attr))
                return trace.Trace(MessageCode.MissingBoundaryConverter, $"Could not find or create {nameof(IBoundaryConverter)} for CLR type '{clrStructType}'");
            var boundaryConverter = new StructConverter(attr.ElementTypeExpression, clrStructType);
            Add(clrStructType, boundaryConverter);
            return boundaryConverter;
        }

        public Result<IValue> LinqToElement(System.Linq.Expressions.Expression parameter, IBoundaryConverter root, CompilationContext context) =>
            // TODO: Detect circular conversion
            TryGetValue(parameter.Type, out var converter)
                ? converter!.LinqToElement(parameter, root, context)
                : TryAddStructConverter(parameter.Type, context)
                    .Bind(converter => converter!.LinqToElement(parameter, root, context));

        public Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction,
                                                 CompilationContext context) =>
            // TODO: Detect circular conversion
            TryGetValue(outputType, out var output)
                ? output!.ElementToLinq(value, outputType, convertFunction, context)
                : TryAddStructConverter(outputType, context)
                    .Bind(converter => converter!.ElementToLinq(value, outputType, convertFunction, context));
    }
}