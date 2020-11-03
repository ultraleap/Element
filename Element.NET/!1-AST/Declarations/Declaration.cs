using System;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    public abstract class Declaration : AstNode
    {
#pragma warning disable 649, 8618, 169
        // ReSharper disable UnassignedField.Global UnusedAutoPropertyAccessor.Local UnusedAutoPropertyAccessor.Global
        [IndirectLiteral(nameof(IntrinsicQualifier))] protected Unnamed _;
        [IndirectLiteral(nameof(Qualifier)), WhitespaceSurrounded] protected Unnamed __;
        [Term] public Identifier Identifier;
        [Optional] public PortList? PortList;
        [Optional] public PortConstraint? ReturnConstraint;
        [IndirectAlternative(nameof(BodyAlternatives)), WhitespaceSurrounded, MultiLine] public object Body;
        // ReSharper restore UnassignedField.Global UnusedAutoPropertyAccessor.Local UnusedAutoPropertyAccessor.Global
#pragma warning restore 649, 8618, 169

        protected abstract string IntrinsicQualifier { get; }
        protected abstract string Qualifier { get; }
        protected abstract Type[] BodyAlternatives { get; }

        public override string ToString() => $"{Identifier} <{GetType().Name}>";

        public Result<IValue> Resolve(IScope scope, Context context)
        {
            if (context.DeclarationStack.Contains(this))
            {
                return context.Trace(EleMessageCode.RecursionNotAllowed, $"{this} has self-referencing implementation");
            }
            
            context.DeclarationStack.Push(this);
            context.TraceStack.Push(this.MakeTraceSite(ToString()));
            var result = Validate(context)
                         .Bind(() =>
                         {
                             context.Aspect?.BeforeDeclaration(this, scope);
                             var resolveResult = ResolveImpl(scope, context);
                             return context.Aspect?.Declaration(this, scope, resolveResult) ?? resolveResult;
                         });
            context.TraceStack.Pop();
            context.DeclarationStack.Pop();
            return result;
        }

        protected abstract Result<IValue> ResolveImpl(IScope scope, Context context);

        protected sealed override void ValidateImpl(ResultBuilder builder, Context context)
        {
            context.DeclarationStack.Push(this);
            context.TraceStack.Push(this.MakeTraceSite(ToString()));
            ValidateDeclaration(builder, context);
            context.TraceStack.Pop();
            context.DeclarationStack.Pop();
        }

        protected abstract void ValidateDeclaration(ResultBuilder builder, Context context);
    }
}