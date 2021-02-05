using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using Element.AST;
using Expression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public class BoundaryMap
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
                _           => Kind.BidirectionalConverter
            };

            public bool AddToCache(BoundaryMap cache)
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

            public Result Resolve(BoundaryMap cache, Context context)
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

        public static BoundaryMap Create(IEnumerable<Mapping> mappings) =>
            mappings.Aggregate(new BoundaryMap(),
                (cache, mapping) =>
                {
                    cache.AddConverter(in mapping);
                    return cache;
                });

        public static BoundaryMap CreateDefault() =>
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

        private BoundaryMap() { }

        public bool AddConverter(in Mapping mapping) => mapping.AddToCache(this);
        
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

        public Result<IValue> LinqToElement(Expression parameter, BoundaryContext context) =>
            GetConverter(parameter.Type, context)
               .Bind(boundaryConverter => boundaryConverter.LinqToElement(parameter, context));

        public Result<Expression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction,
            BoundaryContext context) =>
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
}