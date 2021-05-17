using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using Element.AST;
using ResultNET;
using Expression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    /// <summary>
    /// Contains Mappings of CLR types to Element types and vice versa.
    /// </summary>
    public class BoundaryMap
    {
        // TODO: Support many-to-many mappings with a default mapping rather than restricting bidirectional mappings to one Element type per CLR type
        
        private readonly Dictionary<Type, IBoundaryConverter> _convertersByClrType = new Dictionary<Type, IBoundaryConverter>();
        private readonly Dictionary<Struct, Type> _elementToClrMappings = new Dictionary<Struct, Type>();
        private readonly List<Mapping> _unresolvedMappings = new List<Mapping>();

        private class AssemblyNameComparer : IEqualityComparer<Assembly>
        {
            bool IEqualityComparer<Assembly>.Equals(Assembly x, Assembly y) => x?.FullName == y?.FullName;

            int IEqualityComparer<Assembly>.GetHashCode(Assembly obj) => obj.FullName.GetHashCode();
        }


        static BoundaryMap()
        {
            AllDistinctNonDynamicAssemblies = AppDomain.CurrentDomain.GetAssemblies()
                                                       .Distinct(new AssemblyNameComparer())
                                                       .Where(asm => !asm.IsDynamic)
                                                       .ToList();

            static void ScanAssemblyForBoundaryStructs(Assembly assembly)
            {
                foreach (var type in assembly.GetTypes().Where(t => Attribute.IsDefined(t, typeof(ElementBoundaryStructAttribute))))
                {
                    var attr = type.GetCustomAttribute<ElementBoundaryStructAttribute>();
                    BoundaryStructTypesWithAttribute[type] = attr;
                    DiscoverBoundaryStruct?.Invoke(type, attr);
                }
            }

            BoundaryStructTypesWithAttribute = new Dictionary<Type, ElementBoundaryStructAttribute>();
            foreach (var assembly in AllDistinctNonDynamicAssemblies) ScanAssemblyForBoundaryStructs(assembly);

            AppDomain.CurrentDomain.AssemblyLoad += (sender, args) =>
            {
                if (args.LoadedAssembly.IsDynamic) return;
                AllDistinctNonDynamicAssemblies.Add(args.LoadedAssembly);
                ScanAssemblyForBoundaryStructs(args.LoadedAssembly);
            };
        }

        private static event Action<Type, ElementBoundaryStructAttribute>? DiscoverBoundaryStruct;

        private static readonly Dictionary<IIntrinsicStructImplementation, Type> _intrinsicTypeDictionary = new Dictionary<IIntrinsicStructImplementation, Type>
        {
            {NumStruct.Instance, typeof(float)},
            {BoolStruct.Instance, typeof(bool)}
        };
        private static List<Assembly> AllDistinctNonDynamicAssemblies { get; }
        private static Dictionary<Type, ElementBoundaryStructAttribute> BoundaryStructTypesWithAttribute { get; }

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


        private BoundaryMap()
        {
            // When discovering any new boundary structs we add a converter to this boundary map instance
            DiscoverBoundaryStruct += (type, attribute) => AddConverter(MakeMappingFromAttribute(type, attribute));
        }

        private static Mapping MakeMappingFromAttribute(Type type, ElementBoundaryStructAttribute attr)
        {
            var boundaryStruct = new BoundaryStructInfo(type, attr.ElementTypeExpression, null);
            return Mapping.Bidirectional(type, boundaryStruct.ElementExpression, StructConverter.FromBoundaryStructInfo(boundaryStruct));
        }
        
        public static BoundaryMap Create(IEnumerable<Mapping> mappings) =>
            mappings.Aggregate(new BoundaryMap(),
                (cache, mapping) =>
                {
                    cache.AddConverter(in mapping);
                    return cache;
                });

        public static BoundaryMap CreateDefault() =>
            Create(BoundaryStructTypesWithAttribute
                  .Select(pair => MakeMappingFromAttribute(pair.Key, pair.Value))
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
                               })))
                   }));

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

        public Result<IValue> LinqToElement(Expression parameter, Context context) =>
            GetConverter(parameter.Type, context)
               .Bind(boundaryConverter => boundaryConverter.LinqToElement(parameter, context));

        public Result<Expression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, Context context) =>
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