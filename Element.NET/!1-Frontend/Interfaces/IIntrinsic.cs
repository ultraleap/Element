namespace Element.AST
{
    public interface IIntrinsic : IValue
    {
        string Location { get; }
    }

    public static class IntrinsicExtensions
    {
        public static TDeclaration? GetDeclaration<TDeclaration>(this IIntrinsic intrinsic, CompilationContext compilationContext)
            where TDeclaration : Declaration =>
            compilationContext.GlobalScope[new Identifier(intrinsic.Location), false, compilationContext] switch
            {
                TDeclaration declaration => (IValue)declaration,
                {} v => compilationContext.LogError(4, $"Found declaration '{intrinsic.Location}' but it is not a {nameof(TDeclaration)}"),
                _ => compilationContext.LogError(7, $"Couldn't find '{intrinsic.Location}'")
            } as TDeclaration;
    }
}