using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    public class PortList : Declared
    {
#pragma warning disable 8618
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public ListOf<Port> Ports { get; private set; }
#pragma warning restore 8618

        public override string ToString() => Ports.ToString();

        protected override void InitializeImpl()
        {
            foreach (var p in Ports.List)
            {
                p.Initialize(Declarer);
            }
        }
        
        public override void Validate(ResultBuilder resultBuilder)
        {
            if (Ports.List.Count <= 0) return;
            
            var distinctPortIdentifiers = new HashSet<string>();
            foreach (var port in Ports.List)
            {
                if (!(port.Identifier is { } id)) continue;
                port.Validate(resultBuilder);
                if (!distinctPortIdentifiers.Add(id))
                {
                    resultBuilder.Append(MessageCode.MultipleDefinitions, $"'{Declarer}' has multiple input definitions of '{id}'");
                }
            }
        }
    }
}