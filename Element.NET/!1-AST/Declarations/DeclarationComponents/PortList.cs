using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public class PortList : AstNode
    {
#pragma warning disable 8618
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public ListOf<Port> Ports { get; private set; }
#pragma warning restore 8618

        public override string ToString() => Ports.ToString();

        protected override void ValidateImpl(ResultBuilder builder, Context context)
        {
            if (Ports.List.Count <= 0) return;
            
            var distinctPortIdentifiers = new HashSet<string>();
            var anyDefaultArgumentsSoFar = false;
            foreach (var port in Ports.List)
            {
                if (port.DefaultArgument != null) anyDefaultArgumentsSoFar = true;
                if (anyDefaultArgumentsSoFar && port.DefaultArgument == null) builder.Append(EleMessageCode.PortListDeclaresDefaultArgumentBeforeNonDefault, $"Default argument for port '{port}' in '{context.DeclarationStack.Peek()}' is unreachable");
                port.Validate(builder, context);
                if (!(port.Identifier is { } id)) continue;
                if (!distinctPortIdentifiers.Add(id.String))
                {
                    builder.Append(EleMessageCode.MultipleDefinitions, $"'{context.DeclarationStack.Peek()}' has duplicate input ports named '{id}'");
                }
            }
        }
    }

    public static class PortListExtensions
    {
        public static Result<IReadOnlyList<ResolvedPort>> ResolveInputConstraints(this PortList? portList, IScope scope, Context context, bool portListIsOptional, bool portsListCanBeVaradic) =>
            portList?.Ports.List
                    .Select(p => p.Resolve(scope, context))
                    .ToResultReadOnlyList()
            ?? (portListIsOptional, portsListCanBeVaradic) switch
            {
                (true, false) => Array.Empty<ResolvedPort>(),
                (_, true) => new Result<IReadOnlyList<ResolvedPort>>(new[] {ResolvedPort.VariadicPort}),
                _ => context.Trace(EleMessageCode.MissingPorts, $"'{context.DeclarationStack.Peek()}' must have a port list")
            };
    }
}