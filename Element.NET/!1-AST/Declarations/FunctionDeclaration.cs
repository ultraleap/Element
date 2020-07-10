using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class IntrinsicFunctionDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => "intrinsic";
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override Type[] BodyAlternatives { get; } = {typeof(Terminal)};

        protected override Result<IValue> ResolveImpl(IScope scope, CompilationContext context) =>
            IntrinsicCache.Get<IntrinsicFunctionImplementation>(Identifier, context)
                          .Accumulate(() => PortList.ResolveInputConstraints(scope, context, true, true),
                              () => ReturnConstraint.ResolveReturnConstraint(scope, context))
                          .Map(t =>
                          {
                              var (functionImpl, inputPorts, returnConstraint) = t;
                              return (IValue) new IntrinsicFunction(functionImpl, inputPorts, returnConstraint, context.CurrentDeclarationLocation);
                          });

        protected override void AdditionalValidation(ResultBuilder builder, CompilationContext context)
        {
            builder.Append(IntrinsicCache.Get<IntrinsicFunctionImplementation>(Identifier, builder.Trace));
            
            if (PortList != null && PortList.Ports.List.Any(port => !port.Identifier.HasValue))
            {
                builder.Append(MessageCode.PortListCannotContainDiscards, $"Intrinsic '{context.CurrentDeclarationLocation}' port list contains discards");
            }
        }
    }
    
    public sealed class ExpressionBodiedFunctionDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override Type[] BodyAlternatives { get; } = {typeof(Binding)};

        protected override Result<IValue> ResolveImpl(IScope scope, CompilationContext context) =>
            PortList.ResolveInputConstraints(scope, context, true, false)
                    .Accumulate(() => ReturnConstraint.ResolveReturnConstraint(scope, context))
                    .Map(t =>
                    {
                        var (inputPort, returnConstraint) = t;
                        return (IValue) new ExpressionBodiedFunction(inputPort, returnConstraint, (Binding)Body, scope, context.CurrentDeclarationLocation);
                    });
    }
    
    public sealed class ScopeBodiedFunctionDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override Type[] BodyAlternatives { get; } = {typeof(FunctionBlock)};
        
        protected override void AdditionalValidation(ResultBuilder builder, CompilationContext context)
        {
            if (PortList != null)
            {
                var distinctLocalIdentifiers = new HashSet<Identifier>();
                foreach (var id in PortList.Ports.List
                                           .Where(p => p.Identifier.HasValue)
                                           .Select(p => p.Identifier!.Value)
                                           .Concat(((FunctionBlock) Body).Items.Select(d => d.Identifier)))
                {
                    if (!distinctLocalIdentifiers.Add(id))
                    {
                        builder.Append(MessageCode.MultipleDefinitions, $"Multiple definitions for '{id}' defined in '{context.CurrentDeclarationLocation}' local scope or arguments");
                    }
                }
            }
        }

        protected override Result<IValue> ResolveImpl(IScope scope, CompilationContext context) =>
            PortList.ResolveInputConstraints(scope, context, true, false)
                    .Accumulate(() => ReturnConstraint.ResolveReturnConstraint(scope, context))
                    .Map(t =>
                    {
                        var (inputPort, returnConstraint) = t;
                        return (IValue) new ScopeBodiedFunction(inputPort, returnConstraint, (FunctionBlock)Body, scope, context.CurrentDeclarationLocation);
                    });
    }
}