using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class IntrinsicFunctionDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => "intrinsic";
        protected override string Qualifier => "function";
        protected override Type[] BodyAlternatives { get; } = {typeof(Nothing)};

        protected override Result<IValue> ResolveImpl(IScope scope, Context context) =>
            IntrinsicImplementationCache.Get<IIntrinsicFunctionImplementation>(Identifier, context)
                                        .Bind(impl => PortList.ResolveInputConstraints(scope, context, true, impl.IsVariadic)
                                                              .Accumulate(() => ReturnConstraint.ResolveReturnConstraint(scope, context))
                                                              .Map(t => (impl, t.Item1, t.Item2)))
                                        .Map(t =>
                                        {
                                            var (functionImpl, inputPorts, returnConstraint) = t;
                                            return (IValue)new IntrinsicFunction(functionImpl, inputPorts, returnConstraint);
                                        });

        protected override void ValidateDeclaration(ResultBuilder builder, Context context)
        {
            builder.Append(IntrinsicImplementationCache.Get<IIntrinsicFunctionImplementation>(Identifier, builder.Context));
            PortList?.Validate(builder, context);
            if (PortList != null && PortList.Ports.List.Any(port => !port.Identifier.HasValue))
            {
                builder.Append(MessageCode.PortListCannotContainDiscards, $"Intrinsic '{context.DeclarationStack.Peek()}' port list contains discards");
            }
            ReturnConstraint?.Validate(builder, context);
        }
    }
    
    public sealed class ExpressionBodiedFunctionDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override Type[] BodyAlternatives { get; } = {typeof(ExpressionBody)};

        protected override Result<IValue> ResolveImpl(IScope scope, Context context) =>
            PortList.ResolveInputConstraints(scope, context, true, false)
                    .Accumulate(() => ReturnConstraint.ResolveReturnConstraint(scope, context))
                    .Map(t =>
                    {
                        var (inputPort, returnConstraint) = t;
                        return (IValue) new ExpressionBodiedFunction(Identifier, inputPort, returnConstraint, (ExpressionBody)Body, scope);
                    });

        protected override void ValidateDeclaration(ResultBuilder builder, Context context)
        {
            PortList?.Validate(builder, context);
            ReturnConstraint?.Validate(builder, context);
            if (Body is ExpressionBody expressionBody) expressionBody.Expression.Validate(builder, context);
        }
    }
    
    public sealed class ScopeBodiedFunctionDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override Type[] BodyAlternatives { get; } = {typeof(FunctionBlock)};
        
        protected override void ValidateDeclaration(ResultBuilder builder, Context context)
        {
            PortList?.Validate(builder, context);
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
                        builder.Append(MessageCode.MultipleDefinitions, $"Multiple definitions for '{id}' defined in function '{context.DeclarationStack.Peek()}' local scope or arguments");
                    }
                }
            }
            
            ReturnConstraint?.Validate(builder, context);
            if (Body is FunctionBlock block)
            {
                block.Validate(builder, context);
                block.ValidateIdentifiers(builder);
            }
        }

        protected override Result<IValue> ResolveImpl(IScope scope, Context context) =>
            PortList.ResolveInputConstraints(scope, context, true, false)
                    .Accumulate(() => ReturnConstraint.ResolveReturnConstraint(scope, context))
                    .Map(t =>
                    {
                        var (inputPort, returnConstraint) = t;
                        return (IValue) new ScopeBodiedFunction(Identifier, inputPort, returnConstraint, (FunctionBlock)Body, scope);
                    });
    }
    
    public class FunctionBlock : FreeformBlock
    {
        public void ValidateIdentifiers(ResultBuilder builder)
        {
            foreach (var decl in Items ?? Enumerable.Empty<Declaration>())
            {
                decl.Identifier.Validate(builder,Array.Empty<Identifier>(), new[] {Parser.ReturnIdentifier});
            }
        }
    }
}