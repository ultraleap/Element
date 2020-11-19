using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class Memberwise : IntrinsicValue, IIntrinsicFunctionImplementation
    {
        private Memberwise()
        {
            Identifier =  new Identifier("memberwise");
        }
        
        public static Memberwise Instance { get; } = new Memberwise();
        public override Identifier Identifier { get; }
        public bool IsVariadic => true;

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) =>
            arguments.Skip(1).Select(arg =>
                                         arg.InnerIs<StructInstance>(out var structInstance)
                                             ? new Result<StructInstance>(structInstance)
                                             : context.Trace(EleMessageCode.ConstraintNotSatisfied, $"Expected a struct instance but got '{arg}' - memberwise can only be applied to struct instances"))
                     .ToResultArray()
                     .Bind(structs =>
                     {
                         if (structs.Length < 1) return context.Trace(EleMessageCode.ConstraintNotSatisfied, "memberwise requires at least 1 struct to apply the given function");

                         var structType = structs[0].DeclaringStruct;
                         if (structs.Any(s => s.DeclaringStruct != structType)) return context.Trace(EleMessageCode.ConstraintNotSatisfied, "memberwise can only be applied to struct instances of the same type");

                         Result<IValue> ApplyFuncToMemberPair(ResolvedPort p) =>
                             structs.Select(inst => inst!.Index(p.Identifier!.Value, context))
                                    .BindEnumerable(fields => arguments[0].Call(fields.ToArray(), context));

                         return structType.InputPorts
                                          .Select(ApplyFuncToMemberPair)
                                          .BindEnumerable(resultFields => StructInstance.Create(structType, resultFields.ToArray(), context)
                                                                                        .Cast<IValue>());
                     });
    }
}