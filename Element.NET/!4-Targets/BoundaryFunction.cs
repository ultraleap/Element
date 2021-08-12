using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;
using ResultNET;

namespace Element.CLR
{
    public interface IBoundaryFunction
    {
        string Name { get; }
        string Path { get; }
        IValue Value { get; }
        SourceInfo SourceInfo { get; }
        SourceContext SourceContext { get; }
        IReadOnlyList<ParameterInfo> Parameters { get; }
        IReadOnlyList<LeafParameterInfo> FlattenedParameters { get; }
        ParameterInfo? GetParameter(string path);
    }

    /// <summary>
    /// Represents a valid boundary function created from Element source. 
    /// </summary>
    public class BoundaryFunction : IBoundaryFunction
    {
        private readonly Dictionary<string, ParameterInfo> _parameterInfosByPath;
        
        public BoundaryFunction(string name, string path, IValue value, SourceInfo sourceInfo, SourceContext sourceContext, IReadOnlyList<ParameterInfo> parameters)
        {
            Name = name;
            Path = path;
            Value = value;
            SourceInfo = sourceInfo;
            SourceContext = sourceContext;
            Parameters = parameters;
            _parameterInfosByPath = new Dictionary<string, ParameterInfo>();
            void RecurseParameter(IEnumerable<ParameterInfo> parameterInfos)
            {
                foreach (var pi in parameterInfos)
                {
                    _parameterInfosByPath[pi.FullPath] = pi;
                    if (pi is StructuredParameterInfo spi)
                        RecurseParameter(spi.Fields);
                }
            }
            RecurseParameter(parameters);
        }
        
        public string Name { get; }
        public string Path { get; }
        public IValue Value { get; }
        public SourceInfo SourceInfo { get; }
        public SourceContext SourceContext { get; }
        public override string ToString() => Path;
        
        public ParameterInfo? GetParameter(string path) => _parameterInfosByPath.TryGetValue(path, out var pi) ? pi : null;
        public IReadOnlyList<ParameterInfo> Parameters { get; }
        public IReadOnlyList<LeafParameterInfo> FlattenedParameters => Parameters.SelectMany(p => p.FlattenedParameters()).ToList();
    }

    public static class BoundaryFunctionExtensions
    {
        public static Result<IBoundaryFunction> ToBoundaryFunction(this ValueWithLocation valueWithLocation, SourceContext sourceContext)
            => valueWithLocation.ToBoundaryFunction(valueWithLocation.Identifier.String, valueWithLocation.FullPath, valueWithLocation, sourceContext);
        
        public static Result<IBoundaryFunction> ToBoundaryFunction(this IValue value, string name, string path, ISourceLocation sourceLocation, SourceContext sourceContext)
            => value.InputPortsToParameterInfos(sourceContext)
                    .Map(parameterInfos => (IBoundaryFunction)new BoundaryFunction(name, path, value, sourceLocation.SourceInfo, sourceContext, parameterInfos));

        
        public static Result<IReadOnlyList<ParameterInfo>> InputPortsToParameterInfos(this IValue value, SourceContext sourceContext) =>
            value.InputPorts
                 .Select(p => p.ToParameterInfo(sourceContext))
                 .ToResultReadOnlyList();
        
        public static Result<ParameterInfo> ToParameterInfo(this ResolvedPort resolvedPort, SourceContext sourceContext)
        {
            var context = sourceContext.MakeContext();
            var idStack = new Stack<Identifier>();
            string IdStackToPath() => string.Join(".", idStack.Reverse()); // Stack needs to be reversed as stacks are last in first out

            Result<ParameterInfo> TopLevelPortToParameter(ResolvedPort topLevelPort) =>
                topLevelPort.DefaultValue(context)
                    .Bind(topLevelPortDefault =>
                                 {
                                     Result<ParameterInfo> PortToParameter(ResolvedPort port, ParameterInfo? parent, IValue portDefaultValue)
                                     {
                                         if (!port.Identifier.HasValue) return context.Trace(ElementMessage.InvalidBoundaryFunction, "Boundary value ports must not contain discards");
                                         var portId = port.Identifier.Value;
                                         var argumentPath = IdStackToPath();
                                         idStack.Push(portId);

                                         Result<ParameterInfo> result;
                                         if (port.ResolvedConstraint.InputPorts.Count < 1) // If there's no fields then we're a number
                                         {
                                             result = portDefaultValue.InnerIs(out Constant constant)
                                                 ? (Result<ParameterInfo>) new LeafParameterInfo(portId.String, argumentPath, parent, port.ResolvedConstraint, constant, sourceContext)
                                                 : context.Trace(ElementMessage.InvalidBoundaryFunction, $"Expected a {nameof(Constant)} but got {portDefaultValue}");
                                         }
                                         else
                                         {
                                             result = portDefaultValue.MemberValues(context).Bind(defaultMemberValues =>
                                             {
                                                 Result<ParameterInfo> FieldToParameterInfo(ResolvedPort field) => PortToParameter(field, parent, defaultMemberValues.FirstOrDefault(defaultField => field.Identifier.Value.Equals(defaultField.Identifier)).Value);
                                                 ParameterInfo MakeStructuredParameterInfo(IReadOnlyList<ParameterInfo> fieldParameterInfos) => new StructuredParameterInfo(portId.String, argumentPath, parent, port.ResolvedConstraint, portDefaultValue, sourceContext, fieldParameterInfos);
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

            return TopLevelPortToParameter(resolvedPort);
        }
    }

    public abstract class ParameterInfo
    {
        private string? _fullPath;

        protected ParameterInfo(string name, string path, ParameterInfo? parent, IValue parameterType, IValue @default, SourceContext sourceContext)
        {
            Name = name;
            Path = path;
            Parent = parent;
            ParameterType = parameterType;
            Default = @default;
            SourceContext = sourceContext;
        }

        public string Name { get; }
        public string? Path { get; }
        public string FullPath => _fullPath ??= string.IsNullOrEmpty(Path) ? Name : $"{Path}.{Name}";
        public ParameterInfo? Parent { get; }
        public IValue ParameterType { get; }
        public IValue Default { get; }
        public abstract int? ConstantSize { get; }
        public bool IsDynamicallySized => !ConstantSize.HasValue;
        public SourceContext SourceContext { get; }
        public abstract IEnumerable<LeafParameterInfo> FlattenedParameters();
        
        public Result<TValue> GetValue<TValue>(IBoundaryFunctionArguments source) => GetValue(source).Bind(v => v.CompileInstance<TValue>(SourceContext.MakeContext()));
        public Result SetValue<TValue>(IBoundaryFunctionArguments source, TValue value)
        {
            var serialized = ConstantSize.HasValue ? new List<float>(ConstantSize.Value) : new List<float>();
            return SourceContext.MakeContext()
                                .SerializeClrInstance(value, serialized)
                                .And(() => source.SetMultiple(FullPath, serialized));
        }

        public abstract Result<IValue> GetValue(IBoundaryFunctionArguments source);
        public abstract Result SetValue(IBoundaryFunctionArguments source, IValue value);
    }

    public class LeafParameterInfo : ParameterInfo
    {
        public LeafParameterInfo(string name, string path, ParameterInfo? parent, IValue parameterType, Constant @default, SourceContext sourceContext)
            : base(name, path, parent, parameterType, @default, sourceContext)
            => Default = @default;
        public new Constant Default { get; }
        public override int? ConstantSize => 1;

        public Result<float> GetFloat(IBoundaryFunctionArguments source) => source.GetSingle(FullPath);
        public Result SetFloat(IBoundaryFunctionArguments source, float value) => source.SetSingle(FullPath, value);
        
        public override Result<IValue> GetValue(IBoundaryFunctionArguments source) => GetFloat(source).Map(f => (IValue)new Constant(f, Default.StructImplementation));

        public override Result SetValue(IBoundaryFunctionArguments source, IValue value)
        {
            var context = Context.CreateFromSourceContext(SourceContext);
            return value.InnerIs(out Constant constant)
                && constant.IsInstanceOfType(ParameterType, context)
                    ? SetFloat(source, constant)
                    : context.Trace(ElementMessage.TypeError, $"Expected {ParameterType} value but got {value} of type {value.TypeOf}");
        }

        public override IEnumerable<LeafParameterInfo> FlattenedParameters()
        {
            yield return this;
        }
    }

    public class StructuredParameterInfo : ParameterInfo
    {
        public StructuredParameterInfo(string name, string path, ParameterInfo? parent, IValue parameterType, IValue @default, SourceContext sourceContext, IReadOnlyList<ParameterInfo> fields)
            : base(name, path, parent, parameterType, @default, sourceContext)
        {
            Fields = fields;
            ConstantSize = Fields.All(f => f.ConstantSize.HasValue)
                ? Fields.Sum(f => f.ConstantSize)
                : null;
        }

        public IReadOnlyList<ParameterInfo> Fields { get; }
        
        public override Result<IValue> GetValue(IBoundaryFunctionArguments source)
        {
            var context = Context.CreateFromSourceContext(SourceContext);
            Result<IValue> GetStructValue(StructuredParameterInfo spi) => spi.Fields.Select(FieldToValue).BindEnumerable(fieldsValues => ParameterType.Call(fieldsValues.ToArray(), context));
            Result<IValue> FieldToValue(ParameterInfo pi) =>
                pi switch
                {
                    LeafParameterInfo lpi       => lpi.GetValue(source),
                    StructuredParameterInfo spi => GetStructValue(spi),
                    _                           => throw new ArgumentOutOfRangeException(nameof(pi), pi, $"Unhandled {nameof(ParameterInfo)} type '{pi}'")
                };
            return GetStructValue(this);
        }

        public override Result SetValue(IBoundaryFunctionArguments source, IValue value)
        {
            var context = Context.CreateFromSourceContext(SourceContext);
            return ParameterType.MatchesConstraint(value, context)
                                .And(() => value.SerializeToFloats(context)
                                                .Map(floats =>
                                                 {
                                                     var resultBuilder = new ResultBuilder(context);
                                                     var flattenedFields = FlattenedParameters().ToArray();
                                                     for (var index = 0; index < flattenedFields.Length; index++)
                                                     {
                                                         var field = flattenedFields[index];
                                                         resultBuilder.Append(field.SetFloat(source, new Constant(floats[index], field.Default.StructImplementation)));
                                                     }

                                                     return resultBuilder.ToResult();
                                                 }));
        }

        public override int? ConstantSize { get; }
        public override IEnumerable<LeafParameterInfo> FlattenedParameters() => Fields.SelectMany(f => f.FlattenedParameters());
    }
}