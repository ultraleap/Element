namespace Element
{
    using System.Linq;
    
    /// <summary>
    /// Calls a function with arguments inferred by name from the instance.
    /// </summary>
    internal class InferIntrinsic : INamedFunction
    {
        public string Name => "infer";
        
        public PortInfo[] Inputs { get; } =
        {
            new PortInfo {Name = "func", Type = FunctionType.Instance},
            new PortInfo {Name = "instance", Type = AnyType.Instance}
        };

        public PortInfo[] Outputs { get; } =
        {
            new PortInfo {Name = "return", Type = AnyType.Instance}
        };

        public IFunction CallInternal(IFunction[] arguments, string name, CompilationContext context) =>
            arguments[0].Call(arguments[0].Inputs.Select(input => arguments[1].Call(input.Name, context)).ToArray(), context);
    }
}