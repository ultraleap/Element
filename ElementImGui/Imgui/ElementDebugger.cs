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
        public class ImGuiDebugAspect : CompilationAspectBase
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
                protected ValueImGui(ImGuiDebugAspect aspect, IValue result)
                {
                    _aspect = aspect;
                    _result = result;
                }

                public abstract void DoGui(GuiState state, Context context);

                protected abstract ISourceLocation? SelectionObject { get; }


                private readonly ImGuiDebugAspect _aspect;
                protected readonly IValue _result;

                protected bool TryGetGuiByValue(IValue value, out ValueImGui gui) => _aspect._guisByResult.TryGetValue(value, out gui);
                protected bool TryGetGuiByLocation(ISourceLocation location, out ValueImGui gui) => _aspect._guisByLocation.TryGetValue(location, out gui);
                
                private string? _uniqueSourceLocationString;
                protected static string UniqueLabel(ValueImGui @this, string label) => $"{label}##{nameof(ValueImGui)}{@this._uniqueSourceLocationString ??= @this.SelectionObject?.MakeTraceSite(label).ToString()}";
                protected string UniqueLabel(string label) => UniqueLabel(this, label);

                protected void SelectionText(GuiState state, string text, IValue? valueHint, Context context)
                {
                    ImGui.Text(text);
                    if (ImGui.IsItemClicked()) { state.Selected = SelectionObject; }

                    if (valueHint != null) AppendValueHints(valueHint, context);
                }

                protected void TreeNode(GuiState state, string label, Action guiWhenOpen, IValue? valueHint, Context context)
                {
                    var flags = ImGuiTreeNodeFlags.OpenOnArrow;
                    if (state.Selected == SelectionObject) flags |= ImGuiTreeNodeFlags.Selected;
                    var open = ImGui.TreeNodeEx(UniqueLabel(label), flags);
                    if (ImGui.IsItemClicked()) { state.Selected = SelectionObject; }

                    if (valueHint != null) AppendValueHints(valueHint, context);

                    if (open)
                    {
                        guiWhenOpen();
                        ImGui.TreePop();
                    }
                }

                public class ExpressionChainDrawer
                {
                    private readonly GuiState _state;
                    private readonly IValue _expressionChainResult;
                    private readonly Context _context;
                    private readonly bool _headerOpen;
                    private int _itemIndex;

                    public ExpressionChainDrawer(GuiState state, ExpressionChain chain, ValueImGui caller, IValue expressionChainResult, Context context)
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

                protected static void AppendValueHints(IValue value, Context context)
                {
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

                protected void DoConstraintsGui(GuiState state, Context context)
                {
                    if (state.ShowConstraintAnnotationGui)
                    {
                        ImGui.Text("Declaration Input Ports");
                        foreach (var port in _result.InputPorts)
                        {
                            DoChildGui(port, state, context);
                        }

                        ImGui.Text("Declaration Return Constraint");
                        DoChildGui(_result.ReturnConstraint, state, context);
                        ImGui.Separator();
                    }
                }

                protected void DoExpressionGui(GuiState state, Expression expression, Context context)
                {
                    switch (expression)
                    {
                    case ExpressionChain c:
                        DoChildGui(_result, state, context); // Expression chain gui is handled entirely by items in the chain
                        break;
                    case Lambda l:
                        SelectionText(state, "<lambda>", _result, context);
                        DoConstraintsGui(state, context);
                        break;
                    case AnonymousBlock b:
                        TreeNode(state, "<anonymous block>", () =>
                        {
                            // TODO: Expand block member guis
                        }, _result, context);

                        break;
                    default: throw new ArgumentOutOfRangeException(nameof(expression));
                    }
                }
            }

            private abstract class ExpressionChainItemImGui : ValueImGui
            {
                protected ExpressionChain Chain { get; }
                protected IValue? ResultOfPreviousInChain { get; }
                protected IScope ScopeBeingResolvedIn { get; }
                private readonly UniqueValueSite<ExpressionChain> _uniqueChainKey;

                protected ExpressionChainItemImGui(ExpressionChain chain, IValue? resultOfPreviousInChain, IScope scopeBeingResolvedIn, IValue result) : base(result)
                {
                    Chain = chain;
                    ResultOfPreviousInChain = resultOfPreviousInChain;
                    ScopeBeingResolvedIn = scopeBeingResolvedIn;
                    _uniqueChainKey = new UniqueValueSite<ExpressionChain>(Chain, scopeBeingResolvedIn);
                }

                public override void DoGui(GuiState state, Context context)
                {
                    void DoGuiForPreviousInChain()
                    {
                        if (ResultOfPreviousInChain != null) DoChildGui(ResultOfPreviousInChain, state, context);
                    }

                    var chainHasSubexpressions = Chain.SubExpressions?.Count > 0;

                    if (chainHasSubexpressions)
                    {
                        if (!state.ExpressionChainDrawers.TryGetValue(_uniqueChainKey, out ExpressionChainDrawer drawer))
                        {
                            drawer = state.ExpressionChainDrawers[_uniqueChainKey] = new ExpressionChainDrawer(state, Chain, this, _result, context);
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

                public LiteralImGui(ExpressionChain expressionChain, IScope scope, Constant literal, IValue result)
                    : base(expressionChain, null, scope, result) =>
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

                public LookupImGui(ExpressionChain chain, Identifier id, IScope startScope, IValue result)
                    : base(chain, null, startScope, result) =>
                    _id = id;

                protected override void DoChainItemGui(GuiState state, Context context) => DoChildGui(_result, state, context);
                protected override string Label => _id.TraceString;
                protected override ISourceLocation? SelectionObject => Chain;
            }

            private class IndexImGui : ExpressionChainItemImGui
            {
                private readonly ExpressionChain.IndexingExpression _indexingExpression;

                public IndexImGui(ExpressionChain chain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression indexingExpression, IValue result)
                    : base(chain, valueBeingIndexed, scope, result) =>
                    _indexingExpression = indexingExpression;

                protected override void DoChainItemGui(GuiState state, Context context)
                {
                    // TODO: Evaluate all of the members of the previous indexed value here?
                    DoChildGui(_result, state, context);
                }

                protected override string Label => _indexingExpression.ToString();
                protected override ISourceLocation? SelectionObject => _indexingExpression;
            }

            private class CallImGui : ExpressionChainItemImGui
            {
                private readonly ExpressionChain.CallExpression _callExpression;
                private readonly IReadOnlyList<IValue> _arguments;

                public CallImGui(ExpressionChain chain, IValue function, IScope scope, ExpressionChain.CallExpression callExpression, IReadOnlyList<IValue> arguments, IValue result)
                    : base(chain, function, scope, result)
                {
                    _callExpression = callExpression;
                    _arguments = arguments;
                }

                protected override void DoChainItemGui(GuiState state, Context context)
                {
                    var functionValue = ResultOfPreviousInChain!.Inner();

                    if (functionValue is Struct)
                    {
                        ImGui.Text("Struct Instance Fields");
                        foreach (var arg in _arguments)
                        {
                            DoChildGui(arg, state, context);
                        }

                        return;
                    }

                    if (functionValue.HasInputs())
                    {
                        ImGui.Text("Call Arguments");
                        foreach (var arg in _arguments)
                        {
                            DoChildGui(arg, state, context);
                        }

                        if (!functionValue.IsIntrinsic())
                        {
                            ImGui.Text("Call Body");
                            DoChildGui(_result, state, context);
                        }
                        else
                        {
                            var intrinsicId = (functionValue as IIntrinsicValue)?.Implementation.Identifier.String ?? "<unknown intrinsic>";
                            ImGui.Text($"Intrinsic Call '{intrinsicId}'");
                        }

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

                public CallArgumentImGui(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope, IValue result) : base(result)
                {
                    _function = function;
                    _argumentExpression = argumentExpression;
                    _port = port;
                    _scope = scope;
                }

                public override void DoGui(GuiState state, Context context) =>
                    TreeNode(state, _port == null
                        ? "<variadic argument>"
                        : _port.Identifier.HasValue
                            ? _port.Identifier.Value.String
                            : "<discarded port>", () =>
                    {
                        DoChildGui(_result, state, context);
                    }, _result, context);

                protected override ISourceLocation? SelectionObject => _argumentExpression;
            }

            private class DeclarationImGui : ValueImGui
            {
                private readonly Declaration _declaration;
                private readonly IScope _declaringScope;

                public DeclarationImGui(Declaration declaration, IScope declaringScope, IValue result) : base(result)
                {
                    _declaration = declaration;
                    _declaringScope = declaringScope;
                }

                public override void DoGui(GuiState state, Context context)
                {
                    if (_result is ValueImGui)
                    {
                        DoConstraintsGui(state, context);
                        DoChildGui(_result, state, context);
                    }
                    else
                    {
                        SelectionText(state, _declaration.Identifier.String, _result, context);
                        DoConstraintsGui(state, context);
                    }
                }

                protected override ISourceLocation? SelectionObject => _declaration;
            }

            private class ExpressionImGui : ValueImGui
            {
                private readonly Expression _expression;
                private readonly IScope _scopeResolvedIn;

                public ExpressionImGui(Expression expression, IScope scopeResolvedIn, IValue result) : base(result)
                {
                    _expression = expression;
                    _scopeResolvedIn = scopeResolvedIn;
                }

                public override void DoGui(GuiState state, Context context) => DoExpressionGui(state, _expression, context);

                protected override ISourceLocation? SelectionObject => _expression;
            }

            private class ScopeBodyImGui : ValueImGui
            {
                private readonly FunctionBlock _block;
                private readonly IScope _scope;

                public ScopeBodyImGui(FunctionBlock block, IScope scope, IValue result) : base(result)
                {
                    _block = block;
                    _scope = scope;
                }

                public override void DoGui(GuiState state, Context context) => DoChildGui(_result, state, context);

                protected override ISourceLocation? SelectionObject => _block;
            }

            private class ErrorImGui : ValueImGui
            {
                private readonly IReadOnlyCollection<ResultMessage> _messages;

                public ErrorImGui(ISourceLocation? errorLocation, IReadOnlyCollection<ResultMessage> messages) : base(ErrorValue.Instance)
                {
                    SelectionObject = errorLocation;
                    _messages = messages;
                }

                public override void DoGui(GuiState state, Context context)
                {
                    ImGui.Text("Error occured - hover this text for details.");
                    if (ImGui.IsItemHovered())
                    {
                        foreach (var msg in _messages)
                        {
                            ImGui.TextWrapped(msg.ToString());
                        }
                    }
                }

                protected override ISourceLocation? SelectionObject { get; }
            }

            private class NyiImGui : ValueImGui
            {
                private readonly string _txt;

                public NyiImGui(string txt, ISourceLocation? sourceLocation, IValue result) : base(result)
                {
                    _txt = txt;
                    SelectionObject = sourceLocation;
                }

                public override void DoGui(GuiState state, Context context)
                {
                    if (state.ShowNyiGui)
                    {
                        ImGui.TextDisabled($"Gui for {_txt} is not implemented yet");
                        if (ImGui.IsItemClicked()) state.Selected = SelectionObject;
                    }
                }

                protected override ISourceLocation? SelectionObject { get; }
            }

            #endregion

            private readonly Dictionary<IValue, ValueImGui> _guisByResult = new Dictionary<IValue, ValueImGui>();
            private readonly Dictionary<ISourceLocation, ValueImGui> _guisByLocation = new Dictionary<ISourceLocation, ValueImGui>();
            
            private void PushGui(Func<IValue, ValueImGui> createValueImGui, ISourceLocation? sourceLocation, Result<IValue> result) =>
                result.Switch((value, messages) =>
                    {
                        var gui = createValueImGui(value);
                        _guisByResult[value] = gui;
                        if (sourceLocation != null) _guisByLocation[sourceLocation] = gui;
                    },
                    messages =>
                    {
                        if (sourceLocation != null) _guisByLocation[sourceLocation] = new ErrorImGui(sourceLocation, messages);
                    });

            public override void Declaration(Declaration declaration, IScope scope, Result<IValue> result) => PushGui(value => new DeclarationImGui(declaration, scope, value), declaration, result);
            public override void Expression(Expression expression, IScope scope, Result<IValue> result) => PushGui(value => new ExpressionImGui(expression, scope, value), expression, result);
            public override void Literal(ExpressionChain expressionChain, IScope scope, Constant constant) => PushGui(_ => new LiteralImGui(expressionChain, scope, constant, constant), expressionChain, new Result<IValue>(constant));
            public override void Lookup(ExpressionChain expressionChain, Identifier id, IScope scope, Result<IValue> result) => PushGui(value => new LookupImGui(expressionChain, id, scope, value), expressionChain, result);
            public override void Index(ExpressionChain expressionChain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression expr, Result<IValue> result) => PushGui(value => new IndexImGui(expressionChain, valueBeingIndexed, scope, expr, value), expr, result);
            public override void Call(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments, Result<IValue> result) => PushGui(value => new CallImGui(expressionChain, function, scope, expression, arguments, value), expression, result);
            public override void CallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope, Result<IValue> result) => PushGui(value => new CallArgumentImGui(function, argumentExpression, port, scope, value), argumentExpression, result);
            public override void ExpressionBody(ExpressionBody expression, IScope scope, Result<IValue> result) => PushGui(value => new ExpressionImGui(expression.Expression, scope, value), expression.Expression, result);
            public override void ScopeBody(FunctionBlock functionBlock, IScope scope, Result<IValue> result) => PushGui(value => new ScopeBodyImGui(functionBlock, scope, value), functionBlock, result);
            public override void InputPort(Port port, Expression? constraintExpression, IScope scope, Result<IValue> result) => PushGui(value => new NyiImGui(nameof(InputPort), constraintExpression, value), constraintExpression, result);
            public override void DefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope, Result<IValue> result) => PushGui(value => new NyiImGui(nameof(DefaultArgument), expressionBody?.Expression, value), expressionBody?.Expression, result);
            public override void ReturnConstraint(PortConstraint? constraint, IScope scope, Result<IValue> result) => PushGui(value => new NyiImGui(nameof(ReturnConstraint), constraint, value), constraint, result);
        } 
        
        private readonly ImGuiDebugAspect.GuiState _guiState = new ImGuiDebugAspect.GuiState();
        
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
            _guiState.DoStateGui();

            var dimensions = ImGui.GetWindowSize();
            
            if (ImGui.BeginMenuBar())
            {
                if (ImGui.BeginMenu("Menu"))
                {
                    ImGui.MenuItem("Flags", null, ref _guiState.GuiStateMenu);
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
                            }, DisplayMessages);
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
                ImGui.BeginChild("Debug Tree", new Vector2(dimensions.X / (_guiState.Selected != null ? 2f : 1f), 0f), true, ImGuiWindowFlags.HorizontalScrollbar);

                if (_hasInputChanged || _resolvedValue == null)
                {
                    arguments.ApplyArgumentChanges();
                    var timespan = new Element.CLR.TimeSpan(_time);
                    _resolvedValue = resolveFunction(boundaryFunction.Value, timespan, currentInputValues, debugContext);
                    _hasInputChanged = false;
                }

                _resolvedValue.Value.Switch((result, _) => // TODO: Don't throw away info messages, display them somewhere
                {
                    if (result is ImGuiDebugAspect.ValueImGui wrapper)
                        wrapper.DoGui(_guiState, debugContext);
                    else
                        ImGui.TextColored(_errorTextColor, "Value is not a Gui wrapper. This is an error in the debugger itself!");
                }, DisplayMessages);

                ImGui.EndChild();
                if (_guiState.Selected != null)
                {
                    ImGui.SameLine();
                    ImGui.BeginChild("Debug Trace", new Vector2(dimensions.X / 2f, 0f), true, ImGuiWindowFlags.HorizontalScrollbar);
                    if (ImGui.Button("Close"))
                    {
                        _guiState.Selected = null;
                        ImGui.EndChild();
                        return;
                    }

                    var stringified = _guiState.Selected.ToString();

                    if (_guiState.Selected is ISourceLocation location)
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
                        ImGui.Text($"Type: {_guiState.Selected.GetType()}");
                    }

                    ImGui.EndChild();
                }
            }

            boundaryFunction.Parameters
                     .Select(p => p.GetValue(arguments))
                     .ToResultArray()
                     .Switch(DebugTree, DisplayMessages);
        }
        
        private static void DisplayMessages(IEnumerable<ResultMessage> messages)
        {
            foreach (var msg in messages)
            {
                ImGui.Text(msg.ToString());
            }
        }

        private static readonly Vector4 _errorTextColor = new Vector4(1.0f, 0f, 0f, 1.0f);
    }
}