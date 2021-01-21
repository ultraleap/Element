using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using Element.AST;
using LExpression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public delegate Result<System.Linq.Expressions.Expression> ConvertFunction(IValue value, Type outputType, ClrBoundaryContext context);
    
    public interface IBoundaryConverter
    {
        Result<IValue> LinqToElement(LExpression parameter, ClrBoundaryContext context);
        Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, ClrBoundaryContext context);
        Result SerializeClrInstance(object clrInstance, ICollection<float> floats, ClrBoundaryContext context);
    }

    public class NumberConverter : IBoundaryConverter
    {
        private NumberConverter(){}
        public static NumberConverter Instance { get; } = new NumberConverter();

        public Result<IValue> LinqToElement(System.Linq.Expressions.Expression parameter, ClrBoundaryContext context) =>
            parameter.Type switch
        {
            {} t when t == typeof(bool) => new NumberInstruction(LExpression.Condition(parameter, LExpression.Constant(1f), LExpression.Constant(0f)), BoolStruct.Instance),
            _ => new NumberInstruction(LExpression.Convert(parameter, typeof(float)))
        };

        public Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, ClrBoundaryContext context) =>
            convertFunction(value, typeof(float), context)
                .Map(convertedValue =>
                          (outputType, convertedValue) switch
                          {
                              (_, {} result) when result.Type == outputType => result, // Correct type from convert, return directly
                              ({} t, {} result) when t == typeof(bool) => LExpression.GreaterThan(result, LExpression.Constant(0f)),
                              (_, {} result) => LExpression.Convert(result, outputType),
                              _ => throw new InternalCompilerException($"Unhandled {nameof(ElementToLinq)} output type")
                          });

        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats, ClrBoundaryContext context) =>
            (clrInstance switch
                {
                    float f => new Result<float>(f),
                    bool b => new Result<float>(b ? 1f : 0f),
                    _ => context.Trace(EleMessageCode.SerializationError, $"{nameof(NumberConverter)} doesn't support serializing '{clrInstance}'")
                })
            .Bind(f =>
            {
                floats.Add(f);
                return Result.Success;
            });

        private class NumberInstruction : Instruction, ICLRExpression
        {
            public NumberInstruction(LExpression parameter, IIntrinsicStructImplementation? typeOverride = default)
                : base(typeOverride) =>
                Parameter = parameter;

            public LExpression Parameter { get; }
            public override IEnumerable<Instruction> Dependent => Array.Empty<Instruction>();
            public LExpression Compile() => Parameter;
            public override string SummaryString => Parameter.ToString();
            // ReSharper disable once PossibleUnintendedReferenceComparison
            public override bool Equals(Instruction other) => other == this;
            public override int GetHashCode() => new {Parameter, InstanceTypeOverride = StructImplementation}.GetHashCode();
        }
    }
    
    [AttributeUsage(AttributeTargets.Struct)]
    public class ElementBoundaryStructAttribute : Attribute
    {
        public string ElementTypeExpression { get; }
        public ElementBoundaryStructAttribute(string elementTypeExpression) => ElementTypeExpression = elementTypeExpression;
    }

    public interface IBoundaryStructInfo
    {
        string ElementExpression { get; }
        Dictionary<string, string> FieldMap { get; }
        Result SerializeClrInstance(object clrInstance, ICollection<float> floats, ClrBoundaryContext context);
    }

    public class BoundaryStructInfo : IBoundaryStructInfo
    {
        private readonly Type _structType;
        private readonly Func<object, IEnumerable<object>> _getFieldsFunc;

        public BoundaryStructInfo(Type structType, string elementExpression, Dictionary<string, string>? fieldMap)
        {
            _structType = structType;
            ElementExpression = elementExpression;
            FieldMap = fieldMap ?? structType
                                   .GetFields()
                                   .ToDictionary(f => f.Name,
                                                 f => $"{char.ToLower(f.Name[0])}{f.Name.Substring(1)}");
            
            // Below LinqExpression is equivalent to this reflection-based implementation of a getter
            // IEnumerable<object> getFields(object input) => FieldMap.Select(f => (object)structType.GetField(f).GetValue(input)).ToArray();
            
            var inputExpr = LExpression.Parameter(typeof(object));
            var parameterAsStructType = LExpression.Convert(inputExpr, structType);
            var getFields = FieldMap.Keys.Select(f => LExpression.Convert(LExpression.Field(parameterAsStructType, structType.GetField(f)), typeof(object)));
            var fieldArrayExpression = LExpression.NewArrayInit(typeof(object), getFields);
            _getFieldsFunc = LExpression.Lambda<Func<object, IEnumerable<object>>>(fieldArrayExpression, false, inputExpr).Compile();
        }

        public string ElementExpression { get; }
        public Dictionary<string, string> FieldMap { get; }
        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats, ClrBoundaryContext context) =>
            _structType.IsInstanceOfType(clrInstance)
                ? _getFieldsFunc(clrInstance).Select(f => context.SerializeClrInstance(f, floats)).Fold()
                : throw new InvalidOperationException($"Expected '{nameof(clrInstance)}' to be of type '{_structType}'");
    }

    public class ExternalBoundaryStructInfo : IBoundaryStructInfo
    {
        public ExternalBoundaryStructInfo(string elementExpression, Dictionary<string, string> fieldMap, Func<object, ICollection<float>, ClrBoundaryContext, Result> serializeFunc)
        {
            ElementExpression = elementExpression;
            FieldMap = fieldMap;
            _serializeFunc = serializeFunc;
        }

        private readonly Func<object, ICollection<float>, ClrBoundaryContext, Result> _serializeFunc;

        public string ElementExpression { get; }
        public Dictionary<string, string> FieldMap { get; }
        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats, ClrBoundaryContext context) => _serializeFunc(clrInstance, floats, context);
    }

    public class StructConverter : IBoundaryConverter
    {
        public static StructConverter FromBoundaryStructInfo(IBoundaryStructInfo boundaryStructInfo) => new StructConverter(boundaryStructInfo);

        private readonly IBoundaryStructInfo _boundaryStructInfo;

        private StructConverter(IBoundaryStructInfo boundaryStructInfo) => _boundaryStructInfo = boundaryStructInfo;

        public Result<IValue> LinqToElement(LExpression parameter, ClrBoundaryContext context) =>
            context.EvaluateExpression(_boundaryStructInfo.ElementExpression)
                   .CastInner<Struct>()
                   .Bind(structDeclaration => StructInstance.Create(structDeclaration, _boundaryStructInfo.FieldMap
                                                                                       .Select(pair => new StructFieldInstruction(parameter, pair.Value))
                                                                                       .ToArray(), context)
                                                            .Cast<IValue>());

        private class StructFieldInstruction : Instruction, ICLRExpression
        {
            private readonly LExpression _parameter;
            private readonly string _clrField;

            public StructFieldInstruction(LExpression parameter, string clrField)
            {
                _parameter = parameter;
                _clrField = clrField;
            }

            public LExpression Compile() => LExpression.PropertyOrField(_parameter, _clrField);
            public override IEnumerable<Instruction> Dependent => Array.Empty<Instruction>();
            public override string SummaryString => $"{_parameter}.{_clrField}";
        }

        public Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, ClrBoundaryContext context)
        {
            var obj = LExpression.Variable(outputType);
            var assigns = new List<LExpression>();
            if (!obj.Type.IsValueType)
            {
                assigns.Add(LExpression.Assign(obj, LExpression.New(outputType)));
            }

            return value.InstanceType(context)
                        .Check(v => value.Members.Count < 1
                                    // TODO: More relevant message code - this error case is always a result of API misuse (using struct converter for non-struct instance)
                                    ? context.Trace(EleMessageCode.InvalidBoundaryData, $"Expected instance of a struct with members but got '{value}'")
                                    : Result.Success)
                        .Bind(instanceType =>
                        {
                            var builder = new ResultBuilder<LExpression>(context, default!);
            
                            foreach (var pair in _boundaryStructInfo.FieldMap)
                            {
                                var memberExpression = LExpression.PropertyOrField(obj, pair.Value);
                                var fieldResult = value.Index(new Identifier(pair.Key), context).Bind(fieldValue => convertFunction(fieldValue, memberExpression.Type, context));
                                builder.Append(in fieldResult);
                                if (!fieldResult.IsSuccess) continue;
                                assigns.Add(LExpression.Assign(memberExpression, fieldResult.ResultOr(default!)));
                            }
            
                            assigns.Add(obj);
                            builder.Result = LExpression.Block(new[] {obj}, assigns);
                            return builder.ToResult();
                        });
        }

        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats, ClrBoundaryContext context) => _boundaryStructInfo.SerializeClrInstance(clrInstance, floats, context);
    }
    
    public class ClrBoundary
    {
        private readonly Dictionary<Type, IBoundaryConverter> _convertersByClrType = new Dictionary<Type, IBoundaryConverter>();
        private readonly Dictionary<Struct, Type> _elementToClrMappings = new Dictionary<Struct, Type>();
        private readonly List<Mapping> _unresolvedMappings = new List<Mapping>();

        private static readonly Dictionary<IIntrinsicStructImplementation, Type> _intrinsicTypeDictionary = new Dictionary<IIntrinsicStructImplementation, Type>
        {
            {NumStruct.Instance, typeof(float)},
            {BoolStruct.Instance, typeof(bool)}
        };
        
        private class AssemblyNameComparer : IEqualityComparer<Assembly>
        {
            bool IEqualityComparer<Assembly>.Equals(Assembly x, Assembly y) => x?.FullName == y?.FullName;

            int IEqualityComparer<Assembly>.GetHashCode(Assembly obj) => obj.FullName.GetHashCode();
        }

        
        private static readonly Lazy<Assembly[]> _allDistinctNonDynamicAssemblies = new Lazy<Assembly[]>(() =>
                                                                                                             AppDomain.CurrentDomain.GetAssemblies()
                                                                                                                      .Distinct(new AssemblyNameComparer())
                                                                                                                      .Where(asm => !asm.IsDynamic)
                                                                                                                      .ToArray());


        private static Dictionary<Type, ElementBoundaryStructAttribute> _boundaryStructTypesWithAttribute { get; } =
            _allDistinctNonDynamicAssemblies.Value
                                            .SelectMany(asm => asm.GetTypes())
                                            .Where(t => Attribute.IsDefined(t, typeof(ElementBoundaryStructAttribute)))
                                            .ToDictionary(t => t, t => t.GetCustomAttribute<ElementBoundaryStructAttribute>());

        public readonly struct Mapping
        {
            public static Mapping Bidirectional(Type clrType, string elementStructExpression, IBoundaryConverter converter) => new Mapping(clrType, elementStructExpression, converter);

            public static Mapping ClrToElement(Type clrType, IBoundaryConverter converter) => new Mapping(clrType, null, converter);

            private enum Kind
            {
                ClrConverter,
                BidirectionalConverter
            }

            private Mapping(Type clrType, string? elementStructExpression, IBoundaryConverter converter)
            {
                ClrType = clrType;
                ElementStructExpression = elementStructExpression;
                BoundaryConverter = converter;
            }

            public Type ClrType { get; }
            public string? ElementStructExpression { get; }
            public IBoundaryConverter BoundaryConverter { get; }

            private Kind MappingKind => (ClrType, ElementStructExpression) switch
            {
                ({ }, null) => Kind.ClrConverter,
                _ => Kind.BidirectionalConverter
            };

            public bool AddToCache(ClrBoundary cache)
            {
                switch (MappingKind)
                {
                    case Kind.ClrConverter:
                        if (cache._convertersByClrType.ContainsKey(ClrType)) return false;
                        cache._convertersByClrType[ClrType] = BoundaryConverter;
                        return true;
                    case Kind.BidirectionalConverter:

                        if (!cache.AddConverter(ClrToElement(ClrType, BoundaryConverter))) return false;
                        cache._unresolvedMappings.Add(this);
                        return true;

                    default: throw new ArgumentOutOfRangeException();
                }
            }

            public Result Resolve(ClrBoundary cache, Context context)
            {
                var expr = ElementStructExpression!;
                var type = ClrType;
                return context.EvaluateExpression(expr)
                              .Bind(value =>
                              {
                                  if (!value.InnerIs(out Struct s)) return context.Trace(EleMessageCode.InvalidExpression, $"'{expr}' did not resolve to a Struct");
                                  cache._elementToClrMappings[s] = type;
                                  return Result.Success;
                              });
            }
        }

        public static ClrBoundary Create(IEnumerable<Mapping> mappings) =>
            mappings.Aggregate(new ClrBoundary(),
                               (cache, mapping) =>
                               {
                                   cache.AddConverter(mapping);
                                   return cache;
                               });

        public static ClrBoundary CreateDefault() =>
            Create(_boundaryStructTypesWithAttribute
                   .Select(pair =>
                   {
                       var type = pair.Key;
                       var attr = pair.Value;
                       var boundaryStruct = new BoundaryStructInfo(type, attr.ElementTypeExpression, null);
                       var converter = StructConverter.FromBoundaryStructInfo(boundaryStruct);
                       return Mapping.Bidirectional(type, boundaryStruct.ElementExpression, converter);
                   })
                   .Concat(new[]
                   {
                       Mapping.Bidirectional(typeof(float), "Num", NumberConverter.Instance),
                       Mapping.Bidirectional(typeof(bool), "Bool", NumberConverter.Instance),
                       Mapping.ClrToElement(typeof(int), NumberConverter.Instance),
                       Mapping.ClrToElement(typeof(double), NumberConverter.Instance),
                       Mapping.ClrToElement(typeof(long), NumberConverter.Instance),
                       Mapping.Bidirectional(typeof(Vector2), "Vector2",
                                             StructConverter.FromBoundaryStructInfo(new ExternalBoundaryStructInfo("Vector2",
                                                                                                                   new Dictionary<string, string>
                                                                                                                   {
                                                                                                                       {"x", "X"},
                                                                                                                       {"y", "Y"}
                                                                                                                   },
                                                                                                                   (v, floats, context) =>
                                                                                                                   {
                                                                                                                       var vec2 = (Vector2) v;
                                                                                                                       floats.Add(vec2.X);
                                                                                                                       floats.Add(vec2.Y);
                                                                                                                       return Result.Success;
                                                                                                                   }))),
                       Mapping.Bidirectional(typeof(Vector3), "Vector3",
                                             StructConverter.FromBoundaryStructInfo(new ExternalBoundaryStructInfo("Vector3",
                                                                                                                   new Dictionary<string, string>
                                                                                                                   {
                                                                                                                       {"x", "X"},
                                                                                                                       {"y", "Y"},
                                                                                                                       {"z", "Z"}
                                                                                                                   },
                                                                                                                   (v, floats, context) =>
                                                                                                                   {
                                                                                                                       var vec3 = (Vector3) v;
                                                                                                                       floats.Add(vec3.X);
                                                                                                                       floats.Add(vec3.Y);
                                                                                                                       floats.Add(vec3.Z);
                                                                                                                       return Result.Success;
                                                                                                                   }))),
                       Mapping.Bidirectional(typeof(Vector4), "Vector4",
                                             StructConverter.FromBoundaryStructInfo(new ExternalBoundaryStructInfo("Vector4",
                                                                                                                   new Dictionary<string, string>
                                                                                                                   {
                                                                                                                       {"x", "X"},
                                                                                                                       {"y", "Y"},
                                                                                                                       {"z", "Z"},
                                                                                                                       {"w", "W"}
                                                                                                                   },
                                                                                                                   (v, floats, context) =>
                                                                                                                   {
                                                                                                                       var vec4 = (Vector4) v;
                                                                                                                       floats.Add(vec4.X);
                                                                                                                       floats.Add(vec4.Y);
                                                                                                                       floats.Add(vec4.Z);
                                                                                                                       floats.Add(vec4.W);
                                                                                                                       return Result.Success;
                                                                                                                   }))),
                       Mapping.Bidirectional(typeof(Matrix4x4), "Matrix4x4",
                                             StructConverter.FromBoundaryStructInfo(new ExternalBoundaryStructInfo("Matrix4x4",
                                                                                                                   new Dictionary<string, string>
                                                                                                                   {
                                                                                                                       {"m00", "M11"},
                                                                                                                       {"m01", "M12"},
                                                                                                                       {"m02", "M13"},
                                                                                                                       {"m03", "M14"},
                                                                                                                       {"m10", "M21"},
                                                                                                                       {"m11", "M22"},
                                                                                                                       {"m12", "M23"},
                                                                                                                       {"m13", "M24"},
                                                                                                                       {"m20", "M31"},
                                                                                                                       {"m21", "M32"},
                                                                                                                       {"m22", "M33"},
                                                                                                                       {"m23", "M34"},
                                                                                                                       {"m30", "M41"},
                                                                                                                       {"m31", "M42"},
                                                                                                                       {"m32", "M43"},
                                                                                                                       {"m33", "M44"},
                                                                                                                   },
                                                                                                                   (m, floats, context) =>
                                                                                                                   {
                                                                                                                       var matrix = (Matrix4x4) m;
                                                                                                                       floats.Add(matrix.M11);
                                                                                                                       floats.Add(matrix.M12);
                                                                                                                       floats.Add(matrix.M13);
                                                                                                                       floats.Add(matrix.M14);
                                                                                                                       floats.Add(matrix.M21);
                                                                                                                       floats.Add(matrix.M22);
                                                                                                                       floats.Add(matrix.M23);
                                                                                                                       floats.Add(matrix.M24);
                                                                                                                       floats.Add(matrix.M31);
                                                                                                                       floats.Add(matrix.M32);
                                                                                                                       floats.Add(matrix.M33);
                                                                                                                       floats.Add(matrix.M34);
                                                                                                                       floats.Add(matrix.M41);
                                                                                                                       floats.Add(matrix.M42);
                                                                                                                       floats.Add(matrix.M43);
                                                                                                                       floats.Add(matrix.M44);
                                                                                                                       return Result.Success;
                                                                                                                   }))),
                       Mapping.Bidirectional(typeof(Complex), "Complex",
                                             StructConverter.FromBoundaryStructInfo(new ExternalBoundaryStructInfo("Complex",
                                                                                                                   new Dictionary<string, string>
                                                                                                                   {
                                                                                                                       {"real", "m_real"},
                                                                                                                       {"imag", "m_imaginary"}
                                                                                                                   },
                                                                                                                   (c, floats, context) =>
                                                                                                                   {
                                                                                                                       var complex = (Complex) c;
                                                                                                                       floats.Add((float) complex.Real);
                                                                                                                       floats.Add((float) complex.Imaginary);
                                                                                                                       return Result.Success;
                                                                                                                   }))),

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
                   }));

        private ClrBoundary() { }

        public bool AddConverter(Mapping mapping) => mapping.AddToCache(this);
        
        public Result<IBoundaryConverter> GetConverter(Type type, Context context) =>
            _convertersByClrType.TryGetValue(type, out var converter)
                ? new Result<IBoundaryConverter>(converter)
                : context.Trace(EleMessageCode.MissingBoundaryConverter, $"No boundary converter for CLR type '{type}'");

        private void ResolveDeferredMappings(Context context)
        {
            foreach (var unresolvedMapping in _unresolvedMappings)
            {
                unresolvedMapping.Resolve(this, context);
            }

            _unresolvedMappings.Clear();
        }

        public Result<IValue> LinqToElement(System.Linq.Expressions.Expression parameter, ClrBoundaryContext context) =>
            GetConverter(parameter.Type, context)
                .Bind(boundaryConverter => boundaryConverter.LinqToElement(parameter, context));

        public Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction,
                                                 ClrBoundaryContext context) =>
            GetConverter(outputType, context)
                .Bind(converter => converter.ElementToLinq(value, outputType, convertFunction, context));

        public Result<Type> ElementToClr(Struct elementType, Context context)
        {
            ResolveDeferredMappings(context);
            if (_elementToClrMappings.TryGetValue(elementType, out var type))
                return new Result<Type>(type);
            else
                return context.Trace(EleMessageCode.UnmappedBoundaryType, $"No C# type mapped for '{elementType}'");
        }

        public Result<Type> ElementToClr(IIntrinsicStructImplementation intrinsicElementType, Context context) =>
            _intrinsicTypeDictionary.TryGetValue(intrinsicElementType, out var clrType)
                ? new Result<Type>(clrType)
                : context.Trace(EleMessageCode.UnmappedBoundaryType, $"No C# type mapped for '{intrinsicElementType.Identifier}'");
    }
    
    // TODO: Make IContext and pass through to a wrapped context
    public class ClrBoundaryContext : Context
    {
        public ClrBoundary ClrBoundary { get; }
        public static ClrBoundaryContext FromContext(Context context, ClrBoundary cache) => new ClrBoundaryContext(context, cache);

        private ClrBoundaryContext(Context context, ClrBoundary cache) : base(context.RootScope, context.CompilerOptions, context.StructuralTuples) => ClrBoundary = cache;

        public Result<IValue> LinqToElement(LExpression parameter) => ClrBoundary.LinqToElement(parameter, this);
        public Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction) => ClrBoundary.ElementToLinq(value, outputType, convertFunction, this);
        public Result<Type> ElementToClr(Struct elementStruct) => ClrBoundary.ElementToClr(elementStruct, this);
        public Result<Type> ElementToClr(IIntrinsicStructImplementation elementStruct) => ClrBoundary.ElementToClr(elementStruct, this);
        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats) =>
            ClrBoundary.GetConverter(clrInstance.GetType(), this)
                       .Bind(converter => converter.SerializeClrInstance(clrInstance, floats, this));
    }
	
    public static class BoundaryContextExtensions
    {
        public static ClrBoundaryContext ToBoundaryContext(this Context context, ClrBoundary cache) => ClrBoundaryContext.FromContext(context, cache);
        public static Result<ClrBoundaryContext> ToDefaultBoundaryContext(this Context context) => context.ToBoundaryContext(ClrBoundary.CreateDefault());
    }
}