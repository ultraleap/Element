using System;
using System.Linq;

namespace Element.AST
{
    public class IntrinsicStructDeclaration : Declaration
    {
        protected override string IntrinsicQualifier { get; } = "intrinsic ";
        protected override string Qualifier { get; } = "struct ";
        protected override Type[] BodyAlternatives { get; } = {typeof(StructBlock), typeof(Nothing)};
        protected override Result<IValue> ResolveImpl(IScope scope, Context context) =>
            IntrinsicImplementationCache.Get<IIntrinsicStructImplementation>(Identifier, context)
                          .Accumulate(() => PortList.ResolveFunctionPortList(scope, true, false, context))
                          .Bind(t =>
                          {
                              var (structImpl, inputPorts) = t;
                              IntrinsicStruct? result = null;
                              
                              Result<IValue> ToIntrinsicStructResult(ResolvedBlock? associatedBlock) => new Result<IValue>(result = new IntrinsicStruct(structImpl, inputPorts, associatedBlock, scope));
                              
                              return Body is StructBlock b
                                         ? b.ResolveBlock(scope, context, () => result).Bind(ToIntrinsicStructResult)
                                         : ToIntrinsicStructResult(null);
                          });
        
        protected override void ValidateDeclaration(ResultBuilder builder, Context context)
        {
            builder.Append(IntrinsicImplementationCache.Get<IIntrinsicStructImplementation>(Identifier, builder.Context));
            if (ReturnConstraint != null)
            {
                builder.Append(EleMessageCode.StructCannotHaveReturnType, $"Struct '{this}' cannot have declared return type");
            }
            
            PortList?.Validate(builder, context);
            if (Body is StructBlock block)
            {
                block.Validate(builder, context);
                block.ValidateIdentifiers(builder, Identifier);
            }

            if (PortList?.Ports.List.Any(port => !port.Identifier.HasValue) ?? false)
            {
                builder.Append(EleMessageCode.PortListCannotContainDiscards, $"Struct '{this}' contains discards");
            }
        }
    }
    
    public class CustomStructDeclaration : Declaration
    {
        protected override string IntrinsicQualifier { get; } = string.Empty;
        protected override string Qualifier { get; } = "struct ";
        protected override Type[] BodyAlternatives { get; } = {typeof(StructBlock), typeof(Nothing)};
        protected override Result<IValue> ResolveImpl(IScope scope, Context context) =>
            PortList.ResolveFunctionPortList(scope, false, false, context)
                    .Bind(inputPorts =>
                    {
                        CustomStruct? customStruct = null;
                        Result<IValue> ToCustomStruct(ResolvedBlock? associatedBlock) => new Result<IValue>(customStruct = new CustomStruct(Identifier, inputPorts, associatedBlock, scope));
                              
                        return Body is StructBlock b
                                   ? b.ResolveBlock(scope, context, () => customStruct).Bind(ToCustomStruct)
                                   : ToCustomStruct(null);
                    });
        
        protected override void ValidateDeclaration(ResultBuilder builder, Context context)
        {
            if (!(PortList?.Ports.List.Count > 0))
            {
                builder.Append(EleMessageCode.MissingPorts, $"Non intrinsic '{this}' must have ports");
            }
            
            if (ReturnConstraint != null)
            {
                builder.Append(EleMessageCode.StructCannotHaveReturnType, $"Struct '{this}' cannot have declared return type");
            }
            
            PortList?.Validate(builder, context);
            if (Body is StructBlock block)
            {
                block.Validate(builder, context);
                block.ValidateIdentifiers(builder, Identifier);
            }

            if (PortList?.Ports.List.Any(port => !port.Identifier.HasValue) ?? false)
            {
                builder.Append(EleMessageCode.PortListCannotContainDiscards, $"Struct '{this}' contains discards");
            }
        }
    }
    
    public class StructBlock : DeclarationBlock
    {
        public void ValidateIdentifiers(ResultBuilder builder, Identifier structIdentifier)
        {
            foreach (var decl in Items)
            {
                decl.Identifier.Validate(builder,new[] {structIdentifier}, Array.Empty<Identifier>());
            }
        }
    }
}