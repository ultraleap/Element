using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class DeclaredFunction : DeclaredItem, IFunction
    {
        protected override string Qualifier => string.Empty; // Functions don't have a qualifier
        protected override System.Type[] BodyAlternatives { get; } = {typeof(ExpressionBody), typeof(Scope), typeof(Terminal)};
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
            public FunctionInstance(IValue[] arguments, IFunction declarer, IScope parent, Func<IScope, CompilationContext, IValue> compileFunc, Func<IValue[], IScope, CompilationContext, IValue> resolveFunc)
            {
                _arguments = arguments;
                _parent = parent;
                _compileFunc = compileFunc;
                _resolveFunc = resolveFunc;
                Inputs = declarer.Inputs.Skip(arguments.Length).ToArray();
                Output = declarer.Output;
                Type = declarer.Type;
                SetRange(arguments.Select((arg, index) => (declarer.Inputs[index].Identifier, arg)));
            }

            private readonly IScope _parent;
            private readonly IValue[] _arguments;
            private readonly Func<IScope, CompilationContext, IValue> _compileFunc;
            private readonly Func<IValue[], IScope, CompilationContext, IValue> _resolveFunc;
            public IType Type { get; }
            public Port[] Inputs { get; }
            public Type Output { get; }

            public IValue Compile(IScope scope, CompilationContext compilationContext) => _compileFunc(scope, compilationContext);

            public IValue Call(IValue[] _, CompilationContext compilationContext) => _resolveFunc(_arguments, this, compilationContext);

            public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                IndexCache(id) ?? (recurse ? _parent[id, true, compilationContext] : null);
        }

        private IValue Resolve(IValue[] arguments, IScope callScope, CompilationContext compilationContext) =>
            this.ResolveCall(arguments, DeclaredInputs, callScope, compilationContext);

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            arguments.Length > 0
                ?  new FunctionInstance(arguments, this, ChildScope ?? ParentScope, Compile, Resolve)
                : Resolve(arguments, ChildScope ?? ParentScope, compilationContext);

        public IValue Compile(IScope scope, CompilationContext compilationContext) =>
            scope.CompileFunction(Body, compilationContext);
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