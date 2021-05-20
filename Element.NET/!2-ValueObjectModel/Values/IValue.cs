using System;
using System.Collections.Generic;
using ResultNET;

namespace Element.AST
{
    /// <summary>
    /// Represents a value in the Element language.
    /// A value could be any of the first-class constructs, e.g. literal numbers, functions, structs, struct instances, constraints.
    /// </summary>
    public interface IValue
    {
        /// <summary>
        /// Get string representation of a Value.
        /// </summary>
        // TODO: Decide on distinction between summary and ToString
        string ToString();
        
        /// <summary>
        /// The type of a value.
        /// This respects struct types in the language as well as types for abstract values such as functions.
        /// e.g.
        /// "5" is "Num"
        /// "Num" is "IntrinsicStruct"
        /// "_(a) = a" is a lambda which has type "ExpressionBodiedFunction"
        /// </summary>
        string TypeOf { get; }
        
        /// <summary>
        /// String which summarises a value.
        /// </summary>
        // TODO: Decide on distinction between summary and ToString
        string SummaryString { get; }
        
        /// <summary>
        /// Call the value with the given arguments.
        /// Will error if the value is not callable (e.g. a function).
        /// </summary>
        Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context);
        
        /// <summary>
        /// Input ports which describe what arguments are valid when calling this value.
        /// Empty array if the value is not callable.
        /// </summary>
        IReadOnlyList<ResolvedPort> InputPorts { get; }
        
        /// <summary>
        /// Return constraint which describes what return values are valid when calling this value.
        /// Nothing constraint if the value is not callable (i.e. calling will always fail at the nothing constraint even if a return value is resolved).
        /// </summary>
        IValue ReturnConstraint { get; }
        
        /// <summary>
        /// Index the value to return a named member value.
        /// Will error if the value is not indexable or the requested member doesn't exist. 
        /// </summary>
        Result<IValue> Index(Identifier id, Context context);
        
        /// <summary>
        /// Members of this value which can be retrieved via indexing.
        /// Empty array if the value is not indexable.
        /// </summary>
        IReadOnlyList<Identifier> Members { get; }
        
        /// <summary>
        /// Determine if another value matches the constraint that this value represents.
        /// Will error if this value does not represent a constraint.
        /// </summary>
        Result MatchesConstraint(IValue value, Context context);
        
        /// <summary>
        /// Get the default value of this type.
        /// Will error if this value is not a type.
        /// </summary>
        Result<IValue> DefaultValue(Context context);
        
        /// <summary>
        /// Serialize this value to a list of instructions.
        /// Will error if this value is not serializable (cannot be represented at the host boundary).
        /// </summary>
        void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context);
        
        /// <summary>
        /// Deserialize a copy of this value using the instruction providing function.
        /// Will error if this value is not serializable (cannot be represented at the host boundary).
        /// </summary>
        Result<IValue> Deserialize(Func<Instruction> nextValue, Context context);

        /// <summary>
        /// Retrieve the type this value is an instance of.
        /// Will error if the value is not a primitive value or struct instance.
        /// </summary>
        Result<IValue> InstanceType(Context context);
        
        bool IsIntrinsicOfType<TIntrinsicImplementation>() where TIntrinsicImplementation : IIntrinsicImplementation;
        bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic);
        
        
    }

    public class ErrorValue : Value
    {
        private ErrorValue(){}
        public static ErrorValue Instance { get; } = new ErrorValue();
    }
}