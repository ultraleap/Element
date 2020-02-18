namespace Element.AST
{
    public interface ICallable : IValue
    {
        IValue Call(CompilationFrame frame, CompilationContext compilationContext);
        Port[] Inputs { get; }
    }

    public static class CallableExtensions
    {
        public static IValue GetArgumentByIndex(this ICallable callable, int index, CompilationFrame frame, CompilationContext context)
        {
            if (callable.Inputs == null) throw new InternalCompilerException($"{callable} does not have inputs");
            var identifier = callable.Inputs[index].Identifier;
            return frame.Get(identifier, context, out var value) ? value : context.LogError(7, $"Couldn't find '{identifier}'");
        }
    }
}