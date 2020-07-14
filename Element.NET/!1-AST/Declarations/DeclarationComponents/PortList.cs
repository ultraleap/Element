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

        protected override void ValidateImpl(ResultBuilder builder, CompilationContext context)
        {
            if (Ports.List.Count <= 0) return;
            
            var distinctPortIdentifiers = new HashSet<string>();
            var anyDefaultArgumentsSoFar = false;
            foreach (var port in Ports.List)
            {
                if (port.DefaultArgument != null) anyDefaultArgumentsSoFar = true;
                if (anyDefaultArgumentsSoFar && port.DefaultArgument == null) builder.Append(MessageCode.PortListDeclaresDefaultArgumentBeforeNonDefault, $"Default argument for port '{port}' in '{context.CurrentDeclarationLocation}' is unreachable");
                port.Validate(builder, context);
                if (!(port.Identifier is { } id)) continue;
                if (!distinctPortIdentifiers.Add(id.String))
                {
                    builder.Append(MessageCode.MultipleDefinitions, $"'{context.CurrentDeclarationLocation}' has duplicate input ports named '{id}'");
                }
            }
        }
    }

    public static class PortListExtensions
    {
        public static Result<IReadOnlyList<ResolvedPort>> ResolveInputConstraints(this PortList? portList, IScope scope, CompilationContext context, bool portListIsOptional, bool portsListCanBeVaradic) =>
            portList?.Ports.List
                    .Select(p => p.Resolve(scope, context))
                    .ToResultReadOnlyList()
            ?? (portListIsOptional, portsListCanBeVaradic) switch
            {
                (true, false) => Array.Empty<ResolvedPort>(),
                (_, true) => new Result<IReadOnlyList<ResolvedPort>>(new[] {ResolvedPort.VariadicPort}),
                _ => context.Trace(MessageCode.MissingPorts, $"'{context.CurrentDeclarationLocation}' must have a port list")
            };
    }
}