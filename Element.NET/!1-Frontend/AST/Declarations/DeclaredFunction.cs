using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class DeclaredFunction : Declaration, IFunction
    {
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
        public override IType Type => FunctionType.Instance;
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        Port[] IFunction.Inputs => DeclaredInputs;
        Type IFunction.Output => DeclaredType;
    }

    public class ExtrinsicFunction : DeclaredFunction, ICompilableFunction
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override List<Identifier> ScopeIdentifierWhitelist { get; } = new List<Identifier> {Parser.ReturnIdentifier};

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = ValidateScopeBody(compilationContext);
            if (Body is Terminal)
            {
                compilationContext.LogError(21, $"Non intrinsic function '{Location}' must have a body");
                success = false;
            }

            return success;
        }

        private class FunctionInstance : ScopeBase, ICompilableFunction
        {
            public FunctionInstance(IValue[] arguments, ICompilableFunction declarer, IScope parent, object body)
            {
                _arguments = arguments;
                _declarer = declarer;
                _parent = parent;
                _body = body switch
                {
                    ExpressionBody b => b, // No need to clone expression bodies
                    Scope scopeBody => scopeBody.Clone(this),
                    _ => throw new InternalCompilerException("Cannot create function instance as function body type is not recognized")
                };

                Inputs = declarer.Inputs.Skip(arguments.Length).ToArray();
                SetRange(arguments.Select((arg, index) => (declarer.Inputs[index].Identifier, arg)));
            }

            private readonly ICompilableFunction _declarer;
            private readonly IScope _parent;
            private readonly IValue[] _arguments;
            private readonly object _body;
            public IType Type => _declarer.Type;
            public Port[] Inputs { get; }
            public Type Output => _declarer.Output;

            public IValue Compile(IScope scope, CompilationContext compilationContext) => scope.CompileFunction(_body, compilationContext);

            public bool IsBeingCompiled
            {
                get => _declarer.IsBeingCompiled;
                set => _declarer.IsBeingCompiled = value;
            }

            public IValue Call(IValue[] _, CompilationContext compilationContext) => this.ResolveCall(_arguments, _declarer.Inputs, this, compilationContext);

            public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                IndexCache(id) ?? (recurse ? _parent[id, true, compilationContext] : null);
        }

        private IValue Resolve(IValue[] arguments, IScope callScope, CompilationContext compilationContext) =>
            this.ResolveCall(arguments, DeclaredInputs, callScope, compilationContext);

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            arguments.Length > 0
                ?  new FunctionInstance(arguments, this, ChildScope ?? ParentScope, Body)
                : Resolve(arguments, ChildScope ?? ParentScope, compilationContext);

        public IValue Compile(IScope scope, CompilationContext compilationContext) =>
            scope.CompileFunction(Body, compilationContext);

        public bool IsBeingCompiled { get; set; }
    }

    public class IntrinsicFunction : DeclaredFunction
    {
        protected override string IntrinsicQualifier => "intrinsic";
        public override bool Validate(CompilationContext compilationContext)
        {
            var success = ImplementingIntrinsic<ICallable>(compilationContext) != null;
            if (!(Body is Terminal))
            {
                compilationContext.LogError(20, $"Intrinsic function '{Location}' cannot have a body");
                success = false;
            }

            return success;
        }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            ImplementingIntrinsic<ICallable>(compilationContext).Call(arguments, compilationContext);
    }
}