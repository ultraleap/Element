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
        
        public override bool Validate(SourceContext sourceContext)
        {
            var success = true;
            if (Ports.List.Count > 0)
            {
                var distinctPortIdentifiers = new HashSet<string>();
                foreach (var port in Ports.List)
                {
                    if (!(port.Identifier is { } id)) continue;
                    success &= port.Validate(sourceContext);
                    if (!distinctPortIdentifiers.Add(id))
                    {
                        sourceContext.LogError(2, $"Cannot add duplicate identifier '{id}'");
                        success = false;
                    }
                }
            }
            return success;
        }
    }
}