using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;
using Element.CLR;

namespace Element
{
    public class BoundaryValue
    {
        private BoundaryValue(string name, string path, SourceContext sourceContext, BoundaryMap boundaryMap, IReadOnlyList<ParameterInfo> parameters, IValue value, SourceInfo sourceInfo)
        {
            Name = name;
            Path = path;
            SourceContext = sourceContext;
            BoundaryMap = boundaryMap;
            Parameters = parameters;
            Value = value;
            SourceInfo = sourceInfo;
            var flattenedParameterInfos = new List<LeafParameterInfo>();
            FlattenedParameters = flattenedParameterInfos;
            
            void WalkParameters(IEnumerable<ParameterInfo> parameters)
            {
                foreach (var param in parameters)
                {
                    switch (param)
                    {
                    case LeafParameterInfo t:
                        flattenedParameterInfos.Add(t);
                        break;
                    case StructuredParameterInfo s:
                        WalkParameters(s.Fields);
                        break;
                    default: throw new ArgumentOutOfRangeException(nameof(param));
                    }
                }
            }
            WalkParameters(Parameters);
        }

        public static Result<BoundaryValue> Create(ValueWithLocation value, SourceContext sourceContext, BoundaryMap boundaryMap)
        {
            var context = Context.CreateFromSourceContext(sourceContext);
            var idStack = new Stack<Identifier>();
            string IdStackToPath() => string.Join(".", idStack.Reverse()); // Stack needs to be reversed as stacks are last in first out

            Result<ParameterInfo> TopLevelPortToParameter(ResolvedPort topLevelPort) =>
                topLevelPort.DefaultValue(context)
                    .Bind(topLevelPortDefault =>
                                 {
                                     Result<ParameterInfo> PortToParameter(ResolvedPort port, ParameterInfo? parent, IValue portDefaultValue)
                                     {
                                         if (!port.Identifier.HasValue) return context.Trace(EleMessageCode.InvalidBoundaryFunction, "Boundary value ports must not contain discards");
                                         var portId = port.Identifier.Value;
                                         var argumentPath = IdStackToPath();
                                         idStack.Push(portId);

                                         Result<ParameterInfo> result;
                                         if (port.ResolvedConstraint.InputPorts.Count < 1) // If there's no fields then we're a number
                                         {
                                             result = portDefaultValue.InnerIs(out Constant constant)
                                                 ? (Result<ParameterInfo>) new LeafParameterInfo(portId.String, argumentPath, parent, port.ResolvedConstraint, constant)
                                                 : context.Trace(EleMessageCode.InvalidBoundaryFunction, $"Expected a {nameof(Constant)} but got {portDefaultValue}");
                                         }
                                         else
                                         {
                                             result = portDefaultValue.MemberValues(context).Bind(defaultMemberValues =>
                                             {
                                                 Result<ParameterInfo> FieldToParameterInfo(ResolvedPort field) => PortToParameter(field, parent, defaultMemberValues.FirstOrDefault(defaultField => field.Identifier.Value.Equals(defaultField.Identifier)).Value);
                                                 ParameterInfo MakeStructuredParameterInfo(IReadOnlyList<ParameterInfo> fieldParameterInfos) => new StructuredParameterInfo(portId.String, argumentPath, parent, port.ResolvedConstraint, portDefaultValue, fieldParameterInfos);
                                                 return port.ResolvedConstraint.InputPorts.Select(FieldToParameterInfo)
                                                            .ToResultReadOnlyList()
                                                            .Map(MakeStructuredParameterInfo);
                                             });
                                         }

                                         
                                         idStack.Pop();
                                         return result;
                                     }
                                     
                                     return PortToParameter(topLevelPort, null, topLevelPortDefault);
                                 });

            return value.InputPorts
                        .Select(TopLevelPortToParameter)
                        .ToResultReadOnlyList()
                        .Map(parameterInfos => new BoundaryValue(value.Identifier.String, value.FullPath, sourceContext, boundaryMap, parameterInfos, value, value.SourceInfo));
        }
        
        public string Name { get; }
        public string Path { get; }
        public IValue Value { get; }
        public SourceInfo SourceInfo { get; }
        public SourceContext SourceContext { get; }
        public BoundaryMap BoundaryMap { get; }
        public IReadOnlyList<ParameterInfo> Parameters { get; }
        public IReadOnlyList<LeafParameterInfo> FlattenedParameters { get; }

        public override string ToString() => Path;
    }

    public abstract class ParameterInfo
    {
        private string? _fullPath;

        protected ParameterInfo(string name, string path, ParameterInfo? parent, IValue parameterType)
        {
            Name = name;
            Path = path;
            Parent = parent;
            ParameterType = parameterType;
        }

        public string Name { get; }
        public string? Path { get; }
        public string FullPath => _fullPath ??= string.IsNullOrEmpty(Path) ? Name : $"{Path}.{Name}";
        public ParameterInfo? Parent { get; }
        public IValue ParameterType { get; }
        public abstract Result<IValue> GetValue(IBoundaryArgumentSource source, Context context);
        public abstract Result SetValue(IBoundaryArgumentSource source, IValue value, Context context);
    }

    public class LeafParameterInfo : ParameterInfo
    {
        public LeafParameterInfo(string name, string path, ParameterInfo? parent, IValue parameterType, Constant @default)
            : base(name, path, parent, parameterType)
        {
            Default = @default;
        }
        
        public Constant Default { get; }
        
        public Result<float> GetValue(IBoundaryArgumentSource source) => source.Get(FullPath);
        public Result SetValue(IBoundaryArgumentSource source, float value) => source.Set(FullPath, value);
        
        public override Result<IValue> GetValue(IBoundaryArgumentSource source, Context context) => GetValue(source).Map(f => (IValue)new Constant(f, Default.StructImplementation));

        public override Result SetValue(IBoundaryArgumentSource source, IValue value, Context context) =>
            value.InnerIs(out Constant constant)
            && constant.IsInstanceOfType(ParameterType, context)
                ? SetValue(source, constant)
                : context.Trace(EleMessageCode.TypeError, $"Expected {ParameterType} value but got {value} of type {value.TypeOf}");
    }

    public class StructuredParameterInfo : ParameterInfo
    {
        public StructuredParameterInfo(string name, string path, ParameterInfo? parent, IValue parameterType, IValue @default, IReadOnlyList<ParameterInfo> fields)
            : base(name, path, parent, parameterType)
        {
            Default = @default;
            Fields = fields;
            var flattenedFields = new List<LeafParameterInfo>();
            FlattenedFields = flattenedFields;

            void WalkFields(IEnumerable<ParameterInfo> parameterInfos)
            {
                foreach (var pi in parameterInfos)
                {
                    switch (pi)
                    {
                    case LeafParameterInfo lpi:
                        flattenedFields.Add(lpi);
                        break;
                    case StructuredParameterInfo spi:
                        WalkFields(spi.Fields);
                        break;
                    }
                }
            }
            WalkFields(Fields);
        }
        
        public IValue Default { get; }
        public IReadOnlyList<ParameterInfo> Fields { get; }
        public IReadOnlyList<LeafParameterInfo> FlattenedFields { get; }
        
        public override Result<IValue> GetValue(IBoundaryArgumentSource source, Context context)
        {
            Result<IValue> GetStructValue(StructuredParameterInfo spi) => spi.Fields.Select(FieldToValue).BindEnumerable(fieldsValues => ParameterType.Call(fieldsValues.ToArray(), context));
            Result<IValue> FieldToValue(ParameterInfo pi) =>
                pi switch
                {
                    LeafParameterInfo lpi       => lpi.GetValue(source, context),
                    StructuredParameterInfo spi => GetStructValue(spi),
                    _                           => throw new ArgumentOutOfRangeException(nameof(pi), pi, $"Unhandled {nameof(ParameterInfo)} type '{pi}'")
                };
            return GetStructValue(this);
        }

        public override Result SetValue(IBoundaryArgumentSource source, IValue value, Context context) =>
            ParameterType.MatchesConstraint(value, context)
                         .And(() => value.SerializeToFloats(context)
                                         .Map(floats =>
                                          {
                                              var resultBuilder = new ResultBuilder(context);
                                              for (var index = 0; index < FlattenedFields.Count; index++)
                                              {
                                                  var field = FlattenedFields[index];
                                                  resultBuilder.Append(field.SetValue(source, new Constant(floats[index], field.Default.StructImplementation)));
                                              }

                                              return resultBuilder.ToResult();
                                          }));
    }
}