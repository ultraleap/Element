using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;
using Element.CLR;

namespace Element
{
    public class BoundaryValue
    {
        private readonly ValueWithLocation _value;
        
        protected BoundaryValue(ValueWithLocation value, SourceContext sourceContext, BoundaryMap boundaryMap, IReadOnlyList<ParameterInfo> parameters)
        {
            _value = value;
            SourceContext = sourceContext;
            BoundaryMap = boundaryMap;
            Parameters = parameters;
        }

        public static Result<BoundaryValue> Create(ValueWithLocation value, SourceContext sourceContext, BoundaryMap boundaryMap) =>
            value.InputPortsToParameterInfos(Context.CreateFromSourceContext(sourceContext))
                 .Map(parameterInfos => new BoundaryValue(value, sourceContext, boundaryMap, parameterInfos));

        public string Name => _value.Identifier.String;
        public string Path => _value.FullPath;
        public IValue Value => _value;
        public SourceInfo SourceInfo => _value.SourceInfo;
        public SourceContext SourceContext { get; }
        public BoundaryMap BoundaryMap { get; }
        public IReadOnlyList<ParameterInfo> Parameters { get; }
        public IReadOnlyList<LeafParameterInfo> FlattenedParameters => Parameters.SelectMany(p => p.FlattenedParameters()).ToList();

        public override string ToString() => Path;
    }

    public abstract class ParameterInfo
    {
        private string? _fullPath;

        protected ParameterInfo(string name, string path, ParameterInfo? parent, IValue parameterType, IValue @default)
        {
            Name = name;
            Path = path;
            Parent = parent;
            ParameterType = parameterType;
            Default = @default;
        }

        public string Name { get; }
        public string? Path { get; }
        public string FullPath => _fullPath ??= string.IsNullOrEmpty(Path) ? Name : $"{Path}.{Name}";
        public ParameterInfo? Parent { get; }
        public IValue ParameterType { get; }
        public IValue Default { get; }
        public abstract IEnumerable<LeafParameterInfo> FlattenedParameters();
        public abstract Result<IValue> GetValue(IBoundaryArgumentSource source, Context context);
        public abstract Result SetValue(IBoundaryArgumentSource source, IValue value, Context context);
    }

    public class LeafParameterInfo : ParameterInfo
    {
        public LeafParameterInfo(string name, string path, ParameterInfo? parent, IValue parameterType, Constant @default)
            : base(name, path, parent, parameterType, @default) =>
            Default = @default;

        public new Constant Default { get; }
        
        public Result<float> GetValue(IBoundaryArgumentSource source) => source.Get(FullPath);
        public Result SetValue(IBoundaryArgumentSource source, float value) => source.Set(FullPath, value);
        
        public override Result<IValue> GetValue(IBoundaryArgumentSource source, Context context) => GetValue(source).Map(f => (IValue)new Constant(f, Default.StructImplementation));

        public override Result SetValue(IBoundaryArgumentSource source, IValue value, Context context) =>
            value.InnerIs(out Constant constant)
            && constant.IsInstanceOfType(ParameterType, context)
                ? SetValue(source, constant)
                : context.Trace(EleMessageCode.TypeError, $"Expected {ParameterType} value but got {value} of type {value.TypeOf}");

        public override IEnumerable<LeafParameterInfo> FlattenedParameters()
        {
            yield return this;
        }
    }

    public class StructuredParameterInfo : ParameterInfo
    {
        public StructuredParameterInfo(string name, string path, ParameterInfo? parent, IValue parameterType, IValue @default, IReadOnlyList<ParameterInfo> fields)
            : base(name, path, parent, parameterType, @default) =>
            Fields = fields;

        public IReadOnlyList<ParameterInfo> Fields { get; }
        
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
                                              var flattenedFields = FlattenedParameters().ToArray();
                                              for (var index = 0; index < flattenedFields.Length; index++)
                                              {
                                                  var field = flattenedFields[index];
                                                  resultBuilder.Append(field.SetValue(source, new Constant(floats[index], field.Default.StructImplementation)));
                                              }

                                              return resultBuilder.ToResult();
                                          }));

        public override IEnumerable<LeafParameterInfo> FlattenedParameters() => Fields.SelectMany(f => f.FlattenedParameters());
    }
}