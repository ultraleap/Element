using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    [Sequence(ParserFlags = ParserFlags.TraceHeader)]
    public sealed class IntrinsicFunctionDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => "intrinsic ";
        protected override string Qualifier => "function ";
        protected override Type[] BodyAlternatives { get; } = {typeof(Nothing)};

        protected override Result<IValue> ResolveImpl(IScope scope, Context context) =>
            IntrinsicImplementationCache.Get<IIntrinsicFunctionImplementation>(Identifier, context)
                                        .Bind(impl => PortList.ResolveFunctionSignature(scope, true, impl.IsVariadic, ReturnConstraint, context)
                                                              .Map(t => (impl, t.Item1, t.Item2)))
                                        .Bind(t =>
                                        {
                                            var (functionImpl, inputPorts, returnConstraint) = t;
                                            var intrinsicFunction = new Result<IValue>(new IntrinsicFunction(functionImpl, inputPorts, returnConstraint));
                                            return PortList == null && !functionImpl.IsVariadic
                                                       ? intrinsicFunction.Bind(fn => fn.Call(Array.Empty<IValue>(), context))
                                                       : intrinsicFunction;
                                        });

        protected override void ValidateDeclaration(ResultBuilder builder, Context context)
        {
            builder.Append(IntrinsicImplementationCache.Get<IIntrinsicFunctionImplementation>(Identifier, builder.Context));
            PortList?.Validate(builder, context);
            if (PortList != null && PortList.Ports.List.Any(port => !port.Identifier.HasValue))
            {
                builder.Append(EleMessageCode.PortListCannotContainDiscards, $"Intrinsic '{this}' port list contains discards");
            }
            ReturnConstraint?.Validate(builder, context);
        }
    }
    
    [Sequence(ParserFlags = ParserFlags.TraceHeader)]
    public sealed class ExpressionBodiedFunctionDeclaration : Declaration
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override Type[] BodyAlternatives { get; } = {typeof(ExpressionBody)};

        protected override Result<IValue> ResolveImpl(IScope scope, Context context) =>
            PortList.ResolveFunctionSignature(scope, true, false, ReturnConstraint, context)
                    .Bind(t =>
                    {
                        var expressionBodiedFunction = new Result<IValue>(new ExpressionBodiedFunction(t.InputPorts, t.ReturnPort, (ExpressionBody)Body, scope));
                        // Call functions with no args immediately
                        return PortList == null
                                   ? expressionBodiedFunction.Bind(nullary => nullary.Call(Array.Empty<IValue>(), context))
                                   : expressionBodiedFunction;
                    });

        protected override void ValidateDeclaration(ResultBuilder builder, Context context)
        {
            PortList?.Validate(builder, context);
            ReturnConstraint?.Validate(builder, context);
            if (Body is ExpressionBody expressionBody) expressionBody.Expression.Validate(builder, context);
        }
    }
    
    [Sequence(ParserFlags = ParserFlags.TraceHeader)]
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
                        builder.Append(EleMessageCode.MultipleDefinitions, $"Multiple definitions for '{id}' defined in function '{this}' local scope or arguments");
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
            PortList.ResolveFunctionSignature(scope, true, false, ReturnConstraint, context)
                    .Bind(t =>
                    {
                        var scopeBodiedFunction = new Result<IValue>(new ScopeBodiedFunction(t.InputPorts, t.ReturnPort, (FunctionBlock) Body, scope));
                        // Call functions with no args immediately
                        return PortList == null
                                   ? scopeBodiedFunction.Bind(nullary => nullary.Call(Array.Empty<IValue>(), context))
                                   : scopeBodiedFunction;
                    });
    }
    
    public class FunctionBlock : FreeformBlock
    {
        public void ValidateIdentifiers(ResultBuilder builder)
        {
            foreach (var decl in Items)
            {
                decl.Identifier.Validate(builder,Array.Empty<Identifier>(), new[] {Parser.ReturnIdentifier});
            }
        }
    }
}