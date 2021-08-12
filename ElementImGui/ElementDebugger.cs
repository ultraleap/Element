using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using Element;
using Element.AST;
using Element.CLR;
using ImGuiNET;
using ResultNET;

namespace ElementImGui
{
    public class ElementDebugger
    {
        public class ImGuiDebugAspect : ICompilationAspect
        {
            public class GuiState
            {
                public readonly Dictionary<UniqueValueSite<ExpressionChain>, ValueImGui.ExpressionChainDrawer> ExpressionChainDrawers = new Dictionary<UniqueValueSite<ExpressionChain>, ValueImGui.ExpressionChainDrawer>();
                
                public object? Selected { get; set; }
                public bool GuiStateMenu = false;
                public bool IndentExpressionChains = false;
                public bool ShowConstraintAnnotationGui = false;
                public bool ShowNyiGui = false;

                public void DoStateGui()
                {
                    if (GuiStateMenu && ImGui.Begin("Debugger Preferences"))
                    {
                        ImGui.Checkbox("Indent Expression Chains", ref IndentExpressionChains);
                        ImGui.Checkbox("Expand Port Expressions", ref ShowConstraintAnnotationGui);
                        ImGui.Checkbox("Display Not Yet Implemented Text", ref ShowNyiGui);
                        ImGui.End();
                    }
                }
            }

            #region ValueGUIs

            public abstract class ValueImGui
            {
                public abstract void Draw(GuiState state, Context context);
                public Result<IValue> Result { get; set; }
                public ValueImGui? Parent { get; set; }
                public List<ValueImGui> Dependents { get; } = new List<ValueImGui>();

                protected abstract ISourceLocation? SelectionObject { get; }

                protected void DrawDependents(GuiState state, Context context)
                {
                    foreach (var dependent in Dependents)
                    {
                        dependent.Draw(state, context);
                    }
                }
                

                private string? _uniqueSourceLocationString;
                protected static string UniqueLabel(ValueImGui @this, string label) => $"{label}##{nameof(ValueImGui)}{@this._uniqueSourceLocationString ??= @this.SelectionObject?.MakeTraceSite(label).ToString()}";
                protected string UniqueLabel(string label) => UniqueLabel(this, label);

                protected void SelectionText(GuiState state, string text, in Result<IValue> valueHint, Context context)
                {
                    ImGui.Text(text);
                    if (ImGui.IsItemClicked()) { state.Selected = SelectionObject; }

                    AppendValueHints(in valueHint, context);
                }

                protected void TreeNode(GuiState state, string label, Action guiWhenOpen, in Result<IValue> valueHint, Context context)
                {
                    var flags = ImGuiTreeNodeFlags.OpenOnArrow;
                    if (state.Selected == SelectionObject) flags |= ImGuiTreeNodeFlags.Selected;
                    var open = ImGui.TreeNodeEx(UniqueLabel(label), flags);
                    if (ImGui.IsItemClicked()) { state.Selected = SelectionObject; }

                    AppendValueHints(in valueHint, context);

                    if (open)
                    {
                        guiWhenOpen();
                        ImGui.TreePop();
                    }
                }

                public class ExpressionChainDrawer
                {
                    private readonly GuiState _state;
                    private readonly Result<IValue> _expressionChainResult;
                    private readonly Context _context;
                    private readonly bool _headerOpen;
                    private int _itemIndex;

                    public ExpressionChainDrawer(GuiState state, ExpressionChain chain, ValueImGui caller, Result<IValue> expressionChainResult, Context context)
                    {
                        _state = state;
                        _expressionChainResult = expressionChainResult;
                        _context = context;
                        Chain = chain;
                        ChainUniqueLabel = UniqueLabel(caller, chain.ToString());

                        _headerOpen = true;
                        /*_headerOpen = ImGui.CollapsingHeader(ChainUniqueLabel);
                        if (ImGui.IsItemHovered()) ImGui.SetTooltip(ChainUniqueLabel);
                        if (ImGui.IsItemClicked()) state.Selected = chain;
                        AppendValueHints(expressionChainResult, context);*/

                        if (_headerOpen)
                        {
                            ImGui.BeginGroup();
                            ImGui.BeginTabBar(ChainUniqueLabel);
                            if (state.IndentExpressionChains) ImGui.Indent();
                        }
                    }

                    public string ChainUniqueLabel { get; }
                    public ExpressionChain Chain { get; }

                    public bool DoTab(ValueImGui tabWrapper, string tabLabel, Action tabGui)
                    {
                        if (!_headerOpen) return true;

                        if (_itemIndex > Chain.SubExpressions!.Count)
                        {
                            throw new InvalidOperationException("Expression chain gui attempted to draw more items than there were subexpressions");
                        }

                        var tabOpen = ImGui.BeginTabItem(UniqueLabel(tabWrapper, tabLabel));
                        if (ImGui.IsItemClicked()) _state.Selected = tabWrapper.SelectionObject;
                        if (_itemIndex == Chain.SubExpressions.Count) AppendValueHints(_expressionChainResult, _context);

                        if (tabOpen)
                        {
                            tabGui();
                            ImGui.EndTabItem();
                        }

                        _itemIndex++;

                        if (_itemIndex > Chain.SubExpressions!.Count)
                        {
                            if (_state.IndentExpressionChains) ImGui.Unindent();
                            ImGui.EndTabBar();
                            ImGui.EndGroup();
                            ImGui.Separator();
                            return true;
                        }

                        return false;
                    }
                }

                protected static void AppendValueHints(in Result<IValue> result, Context context)
                {
                    if (!result.TryGetValue(out var value)) return;
                    
                    ImGui.SameLine();
                    ImGui.TextColored(new Vector4(0.8f), $" <{value.TypeOf}>");
                    ImGui.SameLine();

                    static string FormatSerialized(float[] floats, IReadOnlyCollection<ResultMessage> messages) => $"({string.Join(",", floats)})";

                    string FormatFunctionOrEmpty(IReadOnlyCollection<ResultMessage> messages) =>
                        value.HasInputs()
                            ? $"({string.Join(",", value.InputPorts)}):{value.ReturnConstraint}"
                            : string.Empty;

                    ImGui.TextColored(new Vector4(0.8f), value.SerializeToFloats(context)
                                                              .Match(FormatSerialized,
                                                                   FormatFunctionOrEmpty));
                }
                
            }

            private abstract class ExpressionChainItemImGui : ValueImGui
            {
                protected ExpressionChain Chain { get; }
                protected IValue? ResultOfPreviousInChain { get; }
                protected IScope ScopeBeingResolvedIn { get; }
                private readonly UniqueValueSite<ExpressionChain> _uniqueChainKey;

                protected ExpressionChainItemImGui(ExpressionChain chain, IValue? resultOfPreviousInChain, IScope scopeBeingResolvedIn)
                {
                    Chain = chain;
                    ResultOfPreviousInChain = resultOfPreviousInChain;
                    ScopeBeingResolvedIn = scopeBeingResolvedIn;
                    _uniqueChainKey = new UniqueValueSite<ExpressionChain>(Chain, scopeBeingResolvedIn);
                }

                public override void Draw(GuiState state, Context context)
                {
                    void DoGuiForPreviousInChain()
                    {
                        if (ResultOfPreviousInChain != null) DrawDependents(state, context);
                    }

                    var chainHasSubexpressions = Chain.SubExpressions?.Count > 0;

                    if (chainHasSubexpressions)
                    {
                        if (!state.ExpressionChainDrawers.TryGetValue(_uniqueChainKey, out ExpressionChainDrawer drawer))
                        {
                            drawer = state.ExpressionChainDrawers[_uniqueChainKey] = new ExpressionChainDrawer(state, Chain, this, Result, context);
                        }

                        DoGuiForPreviousInChain();
                        if (drawer.DoTab(this, Label, () => DoChainItemGui(state, context)))
                        {
                            state.ExpressionChainDrawers.Remove(_uniqueChainKey);
                        }
                    }
                    else
                    {
                        DoGuiForPreviousInChain();
                        DoChainItemGui(state, context);
                    }
                }

                protected abstract void DoChainItemGui(GuiState state, Context context);
                protected abstract string Label { get; }
            }

            private class LiteralImGui : ExpressionChainItemImGui
            {
                private readonly Constant _literal;

                public LiteralImGui(ExpressionChain expressionChain, IScope scope, Constant literal)
                    : base(expressionChain, null, scope) =>
                    _literal = literal;

                protected override void DoChainItemGui(GuiState state, Context context)
                {
                    ImGui.Text(_literal.ToString());
                    ImGui.SameLine();
                    ImGui.TextColored(new Vector4(0.8f), $" <{_literal.TypeOf}>");
                }

                protected override string Label => _literal.ToString();

                protected override ISourceLocation? SelectionObject => Chain;
            }

            private class LookupImGui : ExpressionChainItemImGui
            {
                private readonly Identifier _id;

                public LookupImGui(ExpressionChain chain, Identifier id, IScope startScope)
                    : base(chain, null, startScope) =>
                    _id = id;

                protected override void DoChainItemGui(GuiState state, Context context) => DrawDependents(state, context);
                protected override string Label => _id.TraceString;
                protected override ISourceLocation? SelectionObject => Chain;
            }

            private class IndexImGui : ExpressionChainItemImGui
            {
                private readonly ExpressionChain.IndexingExpression _indexingExpression;

                public IndexImGui(ExpressionChain chain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression indexingExpression)
                    : base(chain, valueBeingIndexed, scope) =>
                    _indexingExpression = indexingExpression;

                protected override void DoChainItemGui(GuiState state, Context context)
                {
                    // TODO: Evaluate all of the members of the previous indexed value here?
                    //DrawGuiByValue(state, Result, context);
                    DrawDependents(state, context);
                }

                protected override string Label => _indexingExpression.ToString();
                protected override ISourceLocation? SelectionObject => _indexingExpression;
            }

            private class CallImGui : ExpressionChainItemImGui
            {
                private readonly ExpressionChain.CallExpression _callExpression;
                private readonly IReadOnlyList<IValue> _arguments;

                public CallImGui(ExpressionChain chain, IValue function, IScope scope, ExpressionChain.CallExpression callExpression, IReadOnlyList<IValue> arguments)
                    : base(chain, function, scope)
                {
                    _callExpression = callExpression;
                    _arguments = arguments;
                }

                protected override void DoChainItemGui(GuiState state, Context context)
                {
                    var functionValue = ResultOfPreviousInChain!.Inner();

                    if (functionValue is Struct type) // This is a constructor call
                    {
                        ImGui.Text($"{type.SummaryString} Constructor");
                        DrawDependents(state, context);

                        return;
                    }

                    if (functionValue.HasInputs())
                    {
                        if (!functionValue.IsIntrinsic())
                        {
                            ImGui.Text($"Function Call '{functionValue.SummaryString}'");
                        }
                        else
                        {
                            var intrinsicId = (functionValue as IIntrinsicValue)?.Implementation.Identifier.String ?? "<unknown intrinsic>";
                            ImGui.Text($"Intrinsic Call '{intrinsicId}'");
                        }
                        
                        DrawDependents(state, context);

                        return;
                    }

                    ImGui.TextDisabled("Unknown call type");
                }

                protected override string Label => _callExpression.ToString();

                protected override ISourceLocation? SelectionObject => _callExpression;
            }

            private class CallArgumentImGui : ValueImGui
            {
                private readonly IValue _function;
                private readonly Expression _argumentExpression;
                private readonly ResolvedPort? _port;
                private readonly IScope _scope;

                public CallArgumentImGui(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope)
                {
                    _function = function;
                    _argumentExpression = argumentExpression;
                    _port = port;
                    _scope = scope;
                }

                public override void Draw(GuiState state, Context context) =>
                    TreeNode(state, _port == null
                        ? "<variadic argument>"
                        : _port.Identifier.HasValue
                            ? _port.Identifier.Value.String
                            : "<discarded port>", () =>
                    {
                        DrawDependents(state, context);
                    }, Result, context);

                protected override ISourceLocation? SelectionObject => _argumentExpression;
            }

            private class DeclarationImGui : ValueImGui
            {
                private readonly Declaration _declaration;
                private readonly IScope _declaringScope;

                public DeclarationImGui(Declaration declaration, IScope declaringScope)
                {
                    _declaration = declaration;
                    _declaringScope = declaringScope;
                }

                public override void Draw(GuiState state, Context context)
                {
                    SelectionText(state, _declaration.Identifier.String, Result, context);
                    DrawDependents(state, context);
                }

                protected override ISourceLocation? SelectionObject => _declaration;
            }

            private class ExpressionImGui : ValueImGui
            {
                private readonly Expression _expression;
                private readonly IScope _scopeResolvedIn;

                public ExpressionImGui(Expression expression, IScope scopeResolvedIn)
                {
                    _expression = expression;
                    _scopeResolvedIn = scopeResolvedIn;
                }

                public override void Draw(GuiState state, Context context)
                {
                    switch (_expression)
                    {
                    case ExpressionChain c:
                        DrawDependents(state, context); // Expression chain gui is handled entirely by items in the chain
                        break;
                    case Lambda l:
                        TreeNode(state, "<lambda>", () =>
                        {
                            DrawDependents(state, context);
                        }, Result, context);
                        break;
                    case AnonymousBlock b:
                        TreeNode(state, "<anonymous block>", () =>
                        {
                            DrawDependents(state, context);
                            // TODO: Expand block member guis
                        }, Result, context);

                        break;
                    default: throw new ArgumentOutOfRangeException(nameof(_expression));
                    }
                }

                protected override ISourceLocation? SelectionObject => _expression;
            }

            private class ScopeBodyImGui : ValueImGui
            {
                private readonly FunctionBlock _block;
                private readonly IScope _scope;

                public ScopeBodyImGui(FunctionBlock block, IScope scope)
                {
                    _block = block;
                    _scope = scope;
                }

                public override void Draw(GuiState state, Context context) => TreeNode(state, "<scope body>", () => DrawDependents(state, context), Result, context);

                protected override ISourceLocation? SelectionObject => _block;
            }

            private class NyiImGui : ValueImGui
            {
                private readonly string _txt;

                public NyiImGui(string txt, ISourceLocation? sourceLocation)
                {
                    _txt = txt;
                    SelectionObject = sourceLocation;
                }

                public override void Draw(GuiState state, Context context)
                {
                    if (state.ShowNyiGui)
                    {
                        ImGui.TextDisabled($"Gui for {_txt} is not implemented yet");
                        if (ImGui.IsItemClicked()) state.Selected = SelectionObject;
                    }
                    DrawDependents(state, context);
                }

                protected override ISourceLocation? SelectionObject { get; }
            }

            #endregion

            public GuiState State { get; } = new GuiState();
            public List<ValueImGui> RootGuis { get; } = new List<ValueImGui>();

            private Stack<ValueImGui> GuiStack { get; } = new Stack<ValueImGui>();

            public void ClearGui()
            {
                RootGuis.Clear();
                GuiStack.Clear();
            }

            public void Draw(Context context)
            {
                foreach (var valueImGui in RootGuis)
                {
                    valueImGui.Draw(State, context);
                }
            }

            private void Push(ValueImGui gui)
            {
                switch (GuiStack.Count)
                {
                case 0: RootGuis.Add(gui);
                    break;
                case > 0:
                {
                    var peeked = GuiStack.Peek();
                    peeked.Dependents.Add(gui);
                    gui.Parent = peeked;
                    break;
                }
                }

                GuiStack.Push(gui);
            }

            private void Pop(in Result<IValue> result) => GuiStack.Pop().Result = result;

            public void BeforeDeclaration(Declaration declaration, IScope scope) => Push(new DeclarationImGui(declaration, scope));
            public void Declaration(Declaration declaration, IScope scope, Result<IValue> result) => Pop(in result);

            public void BeforeExpression(Expression expression, IScope scope) => Push(new ExpressionImGui(expression, scope));
            public void Expression(Expression expression, IScope scope, Result<IValue> result) => Pop(in result);
            
            public void Literal(ExpressionChain expressionChain, IScope scope, Constant constant)
            {
                if (GuiStack.Count > 0)
                {
                    var peeked = GuiStack.Peek();
                    var literalGui = new LiteralImGui(expressionChain, scope, constant);
                    peeked.Dependents.Add(literalGui);
                    literalGui.Parent = peeked;
                }
            }

            public void BeforeLookup(ExpressionChain expressionChain, Identifier id, IScope scope) => Push(new LookupImGui(expressionChain, id, scope));
            public void Lookup(ExpressionChain expressionChain, Identifier id, IScope scope, Result<IValue> result) => Pop(in result);
            
            public void BeforeIndex(ExpressionChain expressionChain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression expr) => Push(new IndexImGui(expressionChain, valueBeingIndexed, scope, expr));
            public void Index(ExpressionChain expressionChain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression expr, Result<IValue> result) => Pop(in result);

            public void BeforeCall(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments) => Push(new CallImGui(expressionChain, function, scope, expression, arguments));
            public void Call(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments, Result<IValue> result) => Pop(in result);

            public void BeforeCallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope) => Push(new CallArgumentImGui(function, argumentExpression, port, scope));
            public void CallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope, Result<IValue> result) => Pop(in result);

            public void BeforeExpressionBody(ExpressionBody expression, IScope scope) => Push(new ExpressionImGui(expression.Expression, scope));
            public void ExpressionBody(ExpressionBody expression, IScope scope, Result<IValue> result) => Pop(in result);

            public void BeforeScopeBody(FunctionBlock functionBlock, IScope scope) => Push(new ScopeBodyImGui(functionBlock, scope));
            public void ScopeBody(FunctionBlock functionBlock, IScope scope, Result<IValue> result) => Pop(in result);
            
            public void BeforeInputPort(Port port, Expression? constraintExpression, IScope scope) => Push(new NyiImGui(nameof(InputPort), constraintExpression));
            public void InputPort(Port port, Expression? constraintExpression, IScope scope, Result<IValue> result) => Pop(in result);
            
            public void BeforeDefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope) => Push(new NyiImGui(nameof(DefaultArgument), expressionBody?.Expression));
            public void DefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope, Result<IValue> result) => Pop(in result);
            
            public void BeforeReturnConstraint(PortConstraint? constraint, IScope scope) => Push(new NyiImGui(nameof(ReturnConstraint), constraint));
            public void ReturnConstraint(PortConstraint? constraint, IScope scope, Result<IValue> result) => Pop(in result);
        }

        private readonly ImGuiDebugAspect _debugAspect = new ImGuiDebugAspect();
        
        private bool _hasInputChanged = true;
        private double _time;
        private int _linesAroundHighlightedInSource = 3;

        private Result<IValue>? _resolvedValue;
        
        private delegate Result<bool> ParameterDrawer(ParameterInfo parameterInfo, IBoundaryFunctionArguments source);

        private delegate bool GuiFunc<TValue>(ref TValue value);
        private static Result<bool> DoParameterGui<TValue>(ParameterInfo info, IBoundaryFunctionArguments source, GuiFunc<TValue> guiFunc) =>
            info.GetValue<TValue>(source)
                .Bind(value => guiFunc(ref value)
                     ? info.SetValue(source, value).Map(() => true) // If set value succeeded, result bool is set to true
                     : false);

        private static readonly Dictionary<string, ParameterDrawer> _parameterDrawers = new Dictionary<string, ParameterDrawer>
        {
            {"Num", (info, source) => DoParameterGui(info, source, (ref float value) => ImGui.InputFloat(info.FullPath, ref value))},
            {"Bool", (info, source) =>
                {
                    bool GuiFunc(ref float value)
                    {
                        var state = value > 0f;
                        var result = ImGui.Checkbox(info.FullPath, ref state);
                        value = state ? 1f : 0f;
                        return result;
                    }

                    return DoParameterGui<float>(info, source, GuiFunc);
                }
            },
            {"Vector2", (info, source) => DoParameterGui(info, source, (ref Vector2 v) => ImGui.InputFloat2(info.FullPath, ref v))},
            {"Vector3", (info, source) => DoParameterGui(info, source, (ref Vector3 v) => ImGui.InputFloat3(info.FullPath, ref v))},
            {"Vector4", (info, source) => DoParameterGui(info, source, (ref Vector4 v) => ImGui.InputFloat4(info.FullPath, ref v))},
            {"Matrix3x3", (info, source) => DoParameterGui(info, source, (ref Matrix3x3 matrix) =>
                {
                    var row0 = new Vector3(matrix.m00, matrix.m01, matrix.m02);
                    var row1 = new Vector3(matrix.m10, matrix.m11, matrix.m12);
                    var row2 = new Vector3(matrix.m20, matrix.m21, matrix.m22);
                    
                    var result = ImGui.InputFloat3($"{info.FullPath} 1", ref row0)
                        || ImGui.InputFloat3($"{info.FullPath} 2", ref row1)
                        || ImGui.InputFloat3($"{info.FullPath} 3", ref row2);

                    matrix.m00 = row0.X;
                    matrix.m01 = row0.Y;
                    matrix.m02 = row0.Z;
                    matrix.m10 = row1.X;
                    matrix.m11 = row1.Y;
                    matrix.m12 = row1.Z;
                    matrix.m20 = row2.X;
                    matrix.m21 = row2.Y;
                    matrix.m22 = row2.Z;
                    return result;
                })
            },
            {"Matrix4x4", (info, source) => DoParameterGui(info, source, (ref Matrix4x4 matrix) =>
                {
                    var row0 = new Vector4(matrix.M11, matrix.M12, matrix.M13, matrix.M14);
                    var row1 = new Vector4(matrix.M21, matrix.M22, matrix.M23, matrix.M24);
                    var row2 = new Vector4(matrix.M31, matrix.M32, matrix.M33, matrix.M34);
                    var row3 = new Vector4(matrix.M41, matrix.M42, matrix.M43, matrix.M44);
                    
                    var result = ImGui.InputFloat4($"{info.FullPath} 1", ref row0)
                        || ImGui.InputFloat4($"{info.FullPath} 2", ref row1)
                        || ImGui.InputFloat4($"{info.FullPath} 3", ref row2)
                        || ImGui.InputFloat4($"{info.FullPath} 4", ref row3);

                    matrix.M11 = row0.X;
                    matrix.M12 = row0.Y;
                    matrix.M13 = row0.Z;
                    matrix.M14 = row0.W;
                    
                    matrix.M21 = row1.X;
                    matrix.M22 = row1.Y;
                    matrix.M23 = row1.Z;
                    matrix.M24 = row1.W;
                    
                    matrix.M31 = row2.X;
                    matrix.M32 = row2.Y;
                    matrix.M33 = row2.Z;
                    matrix.M34 = row2.W;
                    
                    matrix.M41 = row3.X;
                    matrix.M42 = row3.Y;
                    matrix.M43 = row3.Z;
                    matrix.M44 = row3.W;
                    return result;
                })
            }
        };

        public delegate Result<IValue> Resolve(IValue function, Element.CLR.TimeSpan time, IValue[] arguments, Context context);
        
        public void Draw(IBoundaryFunctionArguments arguments, Resolve resolveFunction)
        {
            var boundaryFunction = arguments.BoundaryFunction;
            var debugContext = Context.CreateFromSourceContext(boundaryFunction.SourceContext);
            debugContext.AddAspect(_debugAspect);
            _debugAspect.State.DoStateGui();

            var dimensions = ImGui.GetWindowSize();
            
            if (ImGui.BeginMenuBar())
            {
                if (ImGui.BeginMenu("Menu"))
                {
                    ImGui.MenuItem("Flags", null, ref _debugAspect.State.GuiStateMenu);
                    ImGui.EndMenu();
                }
                ImGui.EndMenuBar();
            }

            if (ImGui.CollapsingHeader("Inputs"))
            {
                foreach (var param in boundaryFunction.Parameters)
                {
                    if (_parameterDrawers.TryGetValue(param.ParameterType.SummaryString, out var drawer))
                    {
                        drawer(param, arguments)
                           .Switch((b, messages) =>
                            {
                                if (b) _hasInputChanged = true;
                            }, ImGuiExtensions.DisplayAsImGuiText);
                    }
                    else
                    {
                        ImGui.Text($"No parameter drawer defined for type {param.ParameterType.SummaryString}");
                    }
                    ImGui.Separator();
                }
                
                var h = GCHandle.Alloc(_time, GCHandleType.Pinned);
                var min = GCHandle.Alloc(0d, GCHandleType.Pinned);
                var max = GCHandle.Alloc(10d, GCHandleType.Pinned);
                if (ImGui.SliderScalar("Time", ImGuiDataType.Double, h.AddrOfPinnedObject(), min.AddrOfPinnedObject(), max.AddrOfPinnedObject(), "%.3f"))
                {
                    _hasInputChanged = true;
                }

                _time = (double)h.Target!;
                h.Free();
                min.Free();
                max.Free();
            }
            
            void DebugTree(IValue[] currentInputValues, IReadOnlyCollection<ResultMessage> messages)
            {
                var guiState = _debugAspect.State;
                
                ImGui.BeginChild("Debug Tree", new Vector2(dimensions.X / (guiState.Selected != null ? 2f : 1f), 0f), true, ImGuiWindowFlags.HorizontalScrollbar);

                if (_hasInputChanged || _resolvedValue == null)
                {
                    arguments.ApplyArgumentChanges();
                    var timespan = new Element.CLR.TimeSpan(_time);
                    _debugAspect.ClearGui();
                    _resolvedValue = resolveFunction(boundaryFunction.Value, timespan, currentInputValues, debugContext);
                    _hasInputChanged = false;
                }

                _debugAspect.Draw(debugContext);

                ImGui.EndChild();
                
                if (guiState.Selected != null)
                {
                    ImGui.SameLine();
                    ImGui.BeginChild("Debug Trace", new Vector2(dimensions.X / 2f, 0f), true, ImGuiWindowFlags.HorizontalScrollbar);
                    if (ImGui.Button("Close"))
                    {
                        guiState.Selected = null;
                        ImGui.EndChild();
                        return;
                    }

                    var stringified = guiState.Selected.ToString();

                    if (guiState.Selected is ISourceLocation location)
                    {
                        // TODO: Don't allocate tonnes of strings every frame
                        var (lineToHighlight, _, indexOnHighlightedLine) = location.SourceInfo.CalculateLineAndColumnFromIndex(location.IndexInSource);
                        var lines = location.SourceInfo.OriginalText.Split(new[]{Environment.NewLine}, StringSplitOptions.None);
                        var maxLinesAround = Math.Max(lineToHighlight - 1, lines.Length - lineToHighlight);
                        
                        ImGui.SameLine();
                        ImGui.SliderInt("Lines around highlighted", ref _linesAroundHighlightedInSource, 0, maxLinesAround, "%d", ImGuiSliderFlags.Logarithmic); // TODO: Why doesn't logarithmic flag work?

                        var traceSite = location.MakeTraceSite(stringified);
                        ImGui.Text($"{traceSite.Source}:{traceSite.Line},{traceSite.Column}");
                        
                        ImGui.BeginChild("Source Text", Vector2.Zero, true, ImGuiWindowFlags.HorizontalScrollbar);
                        var lineIdx = 1;
                        using var reader = new StringReader(location.SourceInfo.OriginalText);
                        string? line;

                        var linesBeforeExpanded = false;
                        var linesAfterExpanded = false;

                        while ((line = reader.ReadLine()) != null)
                        {
                            var isLineToHighlight = lineToHighlight == lineIdx;
                            var color = new Vector4(1.0f, 1.0f, 1.0f, isLineToHighlight ? 1.0f : 0.6f);

                            var isInBeforeHeader = lineIdx < lineToHighlight - _linesAroundHighlightedInSource;
                            var isInAfterHeader = lineIdx > lineToHighlight + _linesAroundHighlightedInSource;
                            var isAroundHighlightedLine = !isInBeforeHeader && !isInAfterHeader;

                            var shouldBeRendered = isAroundHighlightedLine
                                                   || (isInBeforeHeader && linesBeforeExpanded)
                                                   || (isInAfterHeader && linesAfterExpanded);

                            if (shouldBeRendered)
                            {
                                ImGui.Text(lineIdx.ToString());
                                var lineNumberRectSize = ImGui.GetItemRectSize();
                                ImGui.SameLine(0f, 10f);
                                ImGui.TextColored(color, line);
                                if (isLineToHighlight)
                                {
                                    ImGui.Dummy(lineNumberRectSize);
                                    ImGui.SameLine(0f, 10f);
                                    ImGui.Text("^".PadLeft(indexOnHighlightedLine));
                                }
                            }

                            lineIdx++;
                        }

                        ImGui.EndChild();
                    }
                    else
                    {
                        ImGui.Text($"String: {stringified}");
                        ImGui.Text($"Type: {guiState.Selected.GetType()}");
                    }

                    ImGui.EndChild();
                }
            }

            boundaryFunction.Parameters
                     .Select(p => p.GetValue(arguments))
                     .ToResultArray()
                     .Switch(DebugTree, ImGuiExtensions.DisplayAsImGuiText);
        }

        private static readonly Vector4 _errorTextColor = new Vector4(1.0f, 0f, 0f, 1.0f);
    }
}