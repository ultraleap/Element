using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using Element.AST;
using LExpression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public delegate System.Linq.Expressions.Expression? ConvertFunction(IValue value, Type outputType, CompilationContext context);
    
    public interface IBoundaryConverter
    {
        IValue LinqToElement(System.Linq.Expressions.Expression parameter, IBoundaryConverter root, CompilationContext compilationContext);
        System.Linq.Expressions.Expression? ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, CompilationContext compilationContext);
    }

    public class NumberConverter : IBoundaryConverter
    {
        private NumberConverter(){}
        public static NumberConverter Instance { get; } = new NumberConverter();

        public IValue LinqToElement(System.Linq.Expressions.Expression parameter, IBoundaryConverter root, CompilationContext compilationContext) =>
            parameter.Type switch
        {
            {} t when t == typeof(bool) => new NumberExpression(LExpression.Condition(parameter, LExpression.Constant(1f), LExpression.Constant(0f)), BoolType.Instance),
            _ => new NumberExpression(LExpression.Convert(parameter, typeof(float)))
        };

        public System.Linq.Expressions.Expression? ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction,
                                                                 CompilationContext compilationContext) =>
            (outputType, convertFunction(value, typeof(float), compilationContext)) switch
            {
                (_, {} result) when result.Type == outputType => result, // Correct type from convert, return directly
                ({} t, {} result) when t == typeof(bool) => LExpression.LessThanOrEqual(result, LExpression.Constant(0f)),
                (_, {} result) => LExpression.Convert(result, outputType),
                (_, _) => null
            };

        private class NumberExpression : Expression, ICLRExpression
        {
            public NumberExpression(LExpression parameter, AST.IType? typeOverride = default)
                : base(typeOverride) =>
                Parameter = parameter;

            public LExpression Parameter { get; }
            public override IEnumerable<Expression> Dependent => Array.Empty<Expression>();
            public LExpression Compile(Func<Expression, LExpression> compileOther) => Parameter;
            protected override string ToStringInternal() => Parameter.ToString();
            public override bool Equals(Expression other) => other == this;
            public override int GetHashCode() => new {Parameter, InstanceTypeOverride}.GetHashCode();
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
            _elementToClrFieldMapping = structType.GetFields().ToDictionary(f => f.Name, f => f.Name);
        }

        public IValue LinqToElement(LExpression parameter, IBoundaryConverter root, CompilationContext compilationContext)
        {
            var result = compilationContext.SourceContext.EvaluateExpression(_elementTypeExpression, out _);
            if (result == CompilationError.Instance)
            {
                return CompilationError.Instance;
            }

            if (!(result is DeclaredStruct declaredStruct))
            {
                return compilationContext.LogError(14, $"{result} is not a struct declaration");
            }
            
            return declaredStruct.CreateInstance(_elementToClrFieldMapping.Select(pair => new MemberExpression(root, parameter, pair.Value)).ToArray());
        }

        private class MemberExpression : AST.IFunction
        {
            private readonly LExpression _parameter;
            private readonly IBoundaryConverter _root;
            private readonly string _clrField;

            public MemberExpression(IBoundaryConverter root, LExpression parameter, string clrField)
            {
                _root = root;
                _parameter = parameter;
                _clrField = clrField;
            }

            AST.IType IValue.Type => AST.FunctionType.Instance;
            IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;
            Port[] IFunctionSignature.Inputs { get; } = Array.Empty<Port>();
            Port IFunctionSignature.Output { get; } = Port.ReturnPort(AnyConstraint.Instance);
            public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
                _root.LinqToElement(LExpression.PropertyOrField(_parameter, _clrField), _root, compilationContext);
        }

        public LExpression? ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, CompilationContext compilationContext)
        {
            var obj = LExpression.Variable(outputType);
            var assigns = new List<LExpression>();
            if (!obj.Type.IsValueType)
            {
                assigns.Add(LExpression.Assign(obj, LExpression.New(outputType)));
            }

            var scope = value as IScope;
            foreach (var pair in _elementToClrFieldMapping)
            {
                var memberExpression = LExpression.PropertyOrField(obj, pair.Value);
                var result = convertFunction(scope?[new Identifier(pair.Key), false, compilationContext], memberExpression.Type, compilationContext);
                assigns.Add(LExpression.Assign(memberExpression, result));
            }

            assigns.Add(obj);
            return LExpression.Block(new[] {obj}, assigns);
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

        private bool TryAddStructConverter(Type clrStructType, out IBoundaryConverter boundaryConverter)
        {
            if (clrStructType.GetCustomAttribute<ElementStructTemplateAttribute>() is {} attr)
            {
                Add(clrStructType, boundaryConverter = new StructConverter(attr.ElementTypeExpression, clrStructType));
                return true;
            }

            boundaryConverter = null;
            return false;
        }

        public IValue LinqToElement(System.Linq.Expressions.Expression parameter, IBoundaryConverter root, CompilationContext compilationContext)
        {
            if (TryGetValue(parameter.Type, out var retval)
                || TryAddStructConverter(parameter.Type, out retval))
            {
                return retval.LinqToElement(parameter, root, compilationContext);
            }

            return compilationContext.LogError(12, $"No {nameof(IBoundaryConverter)} for CLR type '{parameter.Type}'");
        }

        public System.Linq.Expressions.Expression? ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction,
                                                                 CompilationContext compilationContext)
        {
            // TODO: Detect circular
            if (TryGetValue(outputType, out var output)
                || TryAddStructConverter(outputType, out output))
            {
                return output.ElementToLinq(value, outputType, convertFunction, compilationContext);
            }

            compilationContext.LogError(12, $"No {nameof(IBoundaryConverter)} for CLR type '{outputType}'");
            return null;
        }
    }
}