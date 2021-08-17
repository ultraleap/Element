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
        public class GuiState
        {
            public Result<IValue>? SelectedResult { get; set; }
            public ISourceLocation? SelectedSourceLocation { get; set; }
            public bool PreferencesWindow = false;
            public bool IndentExpressionChains = false;
            public bool ShowConstraintAnnotationGui = false;
            public bool ShowNyiGui = false;

            public void DrawPreferencesWindow()
            {
                if (PreferencesWindow && ImGui.Begin("Debugger Preferences"))
                {
                    ImGui.Checkbox("Indent Expression Chains", ref IndentExpressionChains);
                    ImGui.Checkbox("Expand Port Expressions", ref ShowConstraintAnnotationGui);
                    ImGui.Checkbox("Display Not Yet Implemented Text", ref ShowNyiGui);
                    ImGui.End();
                }
            }
                
            public void DrawMenuBar()
            {
                if (ImGui.BeginMenuBar())
                {
                    if (ImGui.BeginMenu("Menu"))
                    {
                        ImGui.MenuItem("Preferences", null, ref PreferencesWindow);
                        ImGui.EndMenu();
                    }
                    ImGui.EndMenuBar();
                }
            }
        }
        
        public class ImGuiDebugAspect : CompilationAspectBase
        {
            #region ValueGUIs

            public abstract class ValueImGui
            {
                public abstract void Draw(GuiState state, Context context);
                public Result<IValue> Result { get; set; }
                public List<ValueImGui> Children { get; } = new List<ValueImGui>();

                protected abstract ISourceLocation? SelectionSourceLocation { get; }
                

                private string? _uniqueSourceLocationString;
                protected static string UniqueLabel(ValueImGui @this, string label) => $"{label}##{nameof(ValueImGui)}{@this._uniqueSourceLocationString ??= @this.SelectionSourceLocation?.MakeTraceSite(label).ToString()}";
                protected string UniqueLabel(string label) => UniqueLabel(this, label);

                protected void SelectionText(GuiState state, string label, in Result<IValue> result, Context context)
                {
                    ImGui.Text(label);
                    if (ImGui.IsItemClicked()) { state.SelectedSourceLocation = SelectionSourceLocation; }
                    
                    AppendValueHints(in result, context);
                }

                protected void TreeNode(GuiState state, string label, Action guiWhenOpen, in Result<IValue> valueHint, Context context)
                {
                    var flags = ImGuiTreeNodeFlags.OpenOnArrow;
                    if (state.SelectedSourceLocation == SelectionSourceLocation) flags |= ImGuiTreeNodeFlags.Selected;
                    var open = ImGui.TreeNodeEx(UniqueLabel(label), flags);
                    if (ImGui.IsItemClicked()) { state.SelectedSourceLocation = SelectionSourceLocation; }

                    AppendValueHints(in valueHint, context);

                    if (open)
                    {
                        guiWhenOpen();
                        ImGui.TreePop();
                    }
                }
            }

            private class CallImGui : ValueImGui
            {
                private readonly IValue _function;
                private readonly ExpressionChain? _chain;
                private readonly IScope? _scope;
                private readonly ExpressionChain.CallExpression? _callExpression;
                private readonly ISourceLocation? _functionBodySourceLocation;
                private readonly IReadOnlyList<IValue> _arguments;

                public static CallImGui CreateFromCallSite(ExpressionChain chain, IValue function, IScope scope, ExpressionChain.CallExpression callExpression, IReadOnlyList<IValue> arguments) =>
                    new CallImGui(chain, function, scope, callExpression, arguments);

                public static CallImGui CreateFromCall(IValue function, IReadOnlyList<IValue> arguments) => new CallImGui(null, function, null, null, arguments);

                private CallImGui(ExpressionChain? chain, IValue function, IScope? scope, ExpressionChain.CallExpression? callExpression, IReadOnlyList<IValue> arguments)
                {
                    _function = function;
                    _functionBodySourceLocation = (_function as Function)?.BodySourceLocation;
                    _chain = chain;
                    _scope = scope;
                    _callExpression = callExpression;
                    _arguments = arguments;
                }

                public override void Draw(GuiState state, Context context)
                {
                    var label = _function switch
                    {
                        Struct s => s.SummaryString,
                        {} when _function.HasInputs() => _function.IsIntrinsic()
                            ? $"Intrinsic Call '{(_function as IIntrinsicValue)?.Implementation.Identifier.String ?? "<unknown intrinsic>"}'"
                            : $"Function Call '{_function.SummaryString}'",
                        _ => "Unknown Call Type"
                    };

                    
                   TreeNode(state, label, () =>
                   {
                       foreach (var child in Children)
                       {
                           child.Draw(state, context);
                       }
                   }, Result, context);
                }

                protected override ISourceLocation? SelectionSourceLocation => _functionBodySourceLocation ?? _callExpression; // TODO: Instead of always selecting in this priority, provide GUI to select call site and body separately
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
                        ImGui.Text(c.ExpressionChainStart.TraceString);
                        if (ImGui.IsItemClicked()) { state.SelectedSourceLocation = SelectionSourceLocation; }

                        foreach (var subExpression in c.SubExpressions ?? (IReadOnlyList<ExpressionChain.SubExpression>)Array.Empty<ExpressionChain.SubExpression>())
                        {
                            ImGui.Text(subExpression.ToString());
                            if (ImGui.IsItemClicked()) { state.SelectedSourceLocation = subExpression; }
                        }
                        
                        break;
                    case Lambda l:
                        SelectionText(state, "<lambda>", Result, context);
                        break;
                    case AnonymousBlock b:
                        SelectionText(state, "<anonymous block>", Result, context);

                        break;
                    default: throw new ArgumentOutOfRangeException(nameof(_expression));
                    }
                }

                protected override ISourceLocation? SelectionSourceLocation => _expression;
            }

            #endregion

            public GuiState State { get; } = new GuiState();

            public void ClearGui()
            {
                _results.Clear();
                _combinedStack.Clear();
                _expressionStack.Clear();
                _callStack.Clear();
                _callsWithoutCallSites.Clear();
                _referenceLocations.Clear();
            }

            public void DrawResultTree(Context context)
            {
                var labelUID = 0; // Ensures that results with same name are unique GUI controls
                
                void DrawResult(string label, Result<IValue> result)
                {
                    result.Switch((value, messages) =>
                    {
                        void DrawTooltip()
                        {
                            if (messages.Count > 0
                                && ImGui.IsItemHovered())
                            {
                                ImGui.BeginTooltip();
                                messages.DisplayAsImGuiText();
                                ImGui.EndTooltip();
                            }
                        }

                        switch (value.Members.Count)
                        {
                        case >1:
                            var flags = ImGuiTreeNodeFlags.OpenOnArrow;
                            if (State.SelectedResult.Equals(result)) flags |= ImGuiTreeNodeFlags.Selected;
                            var nodeOpen = ImGui.TreeNodeEx($"{label}##{labelUID++}", flags);
                            if (ImGui.IsItemClicked()) { State.SelectedResult = result; }

                            DrawTooltip();
                            AppendValueHints(in result, context);

                            if (nodeOpen)
                            {
                                foreach (var member in value.Members)
                                {
                                    DrawResult(member.String, value.Index(member, context));
                                }

                                ImGui.TreePop();
                            }

                            break;
                        case 0 or 1:
                            ImGui.Text(label);
                            if (ImGui.IsItemClicked()) { State.SelectedResult = result; }

                            DrawTooltip();
                            AppendValueHints(in result, context);
                            break;
                        default: throw new ArgumentOutOfRangeException(nameof(value));
                        }
                    }, ErrorTextWithTooltip);
                }


                foreach (var result in _results)
                {
                    DrawResult("result", result);
                }
            }
            
            public void DrawExpressionTree(Context context)
            {
                if (!State.SelectedResult.HasValue) return;
                ImGui.NewLine();
                ImGui.Text("Expression Tree");
                ImGui.Spacing();
                ImGui.SameLine();
                if (ImGui.Button("Close"))
                {
                    State.SelectedResult = null;
                    return;
                }

                if (_referenceLocations.TryGetValue(State.SelectedResult.Value, out var gui))
                {
                    foreach (var valueImGui in gui)
                    {
                        valueImGui.Draw(State, context);
                    }
                }
                else
                {
                    ImGui.TextColored(_errorTextColor, $"Result selected has no associated gui");
                }
            }

            private static void AppendValueHints(in Result<IValue> result, Context context)
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
            
            private readonly List<Result<IValue>> _results = new List<Result<IValue>>();
            private readonly Stack<ValueImGui> _combinedStack = new Stack<ValueImGui>();
            private readonly Stack<ExpressionImGui> _expressionStack = new Stack<ExpressionImGui>();
            private readonly Stack<CallImGui> _callStack = new Stack<CallImGui>();
            private readonly Stack<CallImGui> _callsWithoutCallSites = new Stack<CallImGui>();
            private readonly Dictionary<Result<IValue>, List<ValueImGui>> _referenceLocations = new Dictionary<Result<IValue>, List<ValueImGui>>();
            
            private void Push<T>(T gui, Stack<T> specificStack) where T : ValueImGui
            {
                _combinedStack.Push(gui);
                specificStack.Push(gui);
            }

            private T Pop<T>(in Result<IValue> result, Stack<T> specificStack) where T : ValueImGui
            {
                if (!_referenceLocations.TryGetValue(result, out var list))
                {
                    list = _referenceLocations[result] = new List<ValueImGui>();
                }

                var popped = _combinedStack.Pop();
                list.Add(popped);
                popped.Result = result;
                if (_combinedStack.Count == 0) _results.Add(result);
                return specificStack.Pop();
            }


            public override void BeforeExpression(Expression expression, IScope scope) => Push(new ExpressionImGui(expression, scope), _expressionStack);
            public override void Expression(Expression expression, IScope scope, Result<IValue> result) => Pop(in result, _expressionStack);
            
            public override void BeforeCallExpression(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments) =>
                Push(CallImGui.CreateFromCallSite(expressionChain, function, scope, expression, arguments), _callStack);

            public override void CallExpression(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments, Result<IValue> result) =>
                Pop(in result, _callStack);

            public override void BeforeCall(IValue function, IReadOnlyList<IValue> arguments)
            {
                if (_callStack.Count >= 1) return;
                // Functions called programmatically by the API rather than by compiling will not have a call expression in source so we need to add a gui for them that doesn't have call site information
                var callGui = CallImGui.CreateFromCall(function, arguments);
                Push(callGui, _callStack);
                _callsWithoutCallSites.Push(callGui);
            }

            public override void Call(IValue function, IReadOnlyList<IValue> arguments, Result<IValue> result)
            {
                static T? PeekIfNotEmpty<T>(Stack<T> stack) where T : class => stack.Count > 0
                    ? stack.Peek()
                    : null;

                var callStackTopItem = PeekIfNotEmpty(_callStack);
                if (callStackTopItem != null && callStackTopItem == PeekIfNotEmpty(_callsWithoutCallSites)) // Both stacks having the same item at top means we're popped back to the same gui
                {
                    Pop(in result, _callStack);
                    _callsWithoutCallSites.Pop();
                }
            }
        }

        private readonly Dictionary<IBoundaryFunctionArguments, BoundaryArgumentDebugger> _boundaryArgumentDebuggers = new Dictionary<IBoundaryFunctionArguments, BoundaryArgumentDebugger>();

        private class BoundaryArgumentDebugger
        {
            public BoundaryArgumentDebugger(SourceContext sourceContext, IBoundaryFunctionArguments arguments, Resolve resolveFunction)
            {
                _arguments = arguments;
                _arguments.ArgumentChangesApplied += () => _hasInputChanged = true;
                _resolveFunction = resolveFunction;
                _context = Context.CreateFromSourceContext(sourceContext);
                _debugAspectContext = Context.CreateFromSourceContext(sourceContext);
                _debugAspectContext.AddAspect(_debugAspect = new ImGuiDebugAspect());
            }

            private readonly Context _context; // Use to perform operations which don't need debug information saved for debugging
            private readonly Context _debugAspectContext; // Use to perform operations which debug information should be generated from
            private readonly ImGuiDebugAspect _debugAspect;
            private GuiState _guiState => _debugAspect.State; 

            public void Draw(float width)
            {
                _guiState.DrawPreferencesWindow();
                DrawParameters();
                
                if (_hasInputChanged || _resolvedArguments == null)
                {
                    _resolvedArguments = _arguments.BoundaryFunction.Parameters
                                                   .Select(p => p.GetValue(_arguments))
                                                   .ToResultArray();
                }

                width = _guiState.SelectedSourceLocation != null
                    ? width * 0.5f
                    : width;

                _resolvedArguments?.Switch((values, _) => DrawHierarchy(values, width), ImGuiExtensions.DisplayAsImGuiText);
                DrawSourceView(width);
            }

            private readonly IBoundaryFunctionArguments _arguments;
            private readonly Resolve _resolveFunction;
            private bool _hasInputChanged = true;
            private double _time;
            private int _linesAroundHighlightedInSource = 3;

            private Result<IValue[]>? _resolvedArguments; 
            private Result<IValue>? _resolvedValue;
            
            #region Parameter Drawers
            
            private delegate bool GuiFunc<TValue>(ref TValue value);
            private static Result<bool> DoParameterGui<TValue>(ParameterInfo info, IBoundaryFunctionArguments source, GuiFunc<TValue> guiFunc) =>
                info.GetValue<TValue>(source)
                    .Bind(value => guiFunc(ref value)
                         ? info.SetValue(source, value).Map(() => true) // If set value succeeded, result bool is set to true
                         : false);

            private delegate Result<bool> ParameterDrawer(ParameterInfo parameterInfo, IBoundaryFunctionArguments source);
            
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
            
            #endregion
            
            private void DrawHierarchy(IValue[] values, float width)
            {
                ImGui.BeginChild("Debug Hierarchy", new Vector2(width, 0f), true, ImGuiWindowFlags.HorizontalScrollbar);

                if (_hasInputChanged || _resolvedValue == null)
                {
                    _arguments.ApplyArgumentChanges();
                    var timespan = new Element.CLR.TimeSpan(_time);
                    _debugAspect.ClearGui();
                    _resolvedValue = _resolveFunction(_arguments.BoundaryFunction, timespan, values, _context, _debugAspectContext);
                    _hasInputChanged = false;
                }

                _debugAspect.DrawResultTree(_context);
                _debugAspect.DrawExpressionTree(_context);

                ImGui.EndChild();
            }
            
            private void DrawParameters()
            {
                if (!ImGui.CollapsingHeader("Inputs")) return;
            
                foreach (var param in _arguments.BoundaryFunction.Parameters)
                {
                    if (_parameterDrawers.TryGetValue(param.ParameterType.SummaryString, out var drawer))
                    {
                        drawer(param, _arguments)
                           .Switch((b, _) =>
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

            private void DrawSourceView(float width)
            {
                if (_guiState.SelectedSourceLocation == null) return;
                
                ImGui.SameLine();
                
                ImGui.BeginChild("Source View", new Vector2(width, 0f), true, ImGuiWindowFlags.HorizontalScrollbar);
                if (ImGui.Button("Close"))
                {
                    _guiState.SelectedSourceLocation = null;
                    ImGui.EndChild();
                    return;
                }

                var location = _guiState.SelectedSourceLocation;
                var stringified = location.ToString();
                
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

                ImGui.EndChild(); // Source text

                ImGui.EndChild(); // Source view
            }
        };
        
        public delegate Result<IValue> Resolve(IBoundaryFunction function, Element.CLR.TimeSpan time, IValue[] arguments, Context context, Context debugContext);
        
        public void Draw(IBoundaryFunctionArguments arguments, Resolve resolveFunction)
        {
            var boundaryFunction = arguments.BoundaryFunction;
            if (!_boundaryArgumentDebuggers.TryGetValue(arguments, out var boundaryArgumentDebugger))
            {
                boundaryArgumentDebugger = _boundaryArgumentDebuggers[arguments] = new BoundaryArgumentDebugger(boundaryFunction.SourceContext, arguments, resolveFunction);
            }
            boundaryArgumentDebugger.Draw(ImGui.GetWindowSize().X);
        }

        private static void ErrorTextWithTooltip(IReadOnlyCollection<ResultMessage> messages)
        {
            ImGui.TextColored(_errorTextColor, $"Error <{messages.Count} messages>");
            if (ImGui.IsItemHovered())
            {
                ImGui.BeginTooltip();
                messages.DisplayAsImGuiText();
                ImGui.EndTooltip();
            }
        }

        private static readonly Vector4 _errorTextColor = new Vector4(1.0f, 0f, 0f, 1.0f);
    }
}