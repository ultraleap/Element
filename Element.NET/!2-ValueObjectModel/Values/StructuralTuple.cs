using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public class StructuralTuple : Value
    {
        private readonly IReadOnlyList<(Identifier Identifier, IValue Value, IValue Type)> _fields;

        private StructuralTuple(IReadOnlyList<(Identifier, IValue, IValue)> fields) => _fields = fields;

        public static Result<IValue> CreateInstance(ResolvedBlock block, Context context) =>
            block.MemberValues( context )
                 .Bind( members =>
                  {
                      var orderedMembers = members.OrderBy( m => m.Identifier.String ).ToArray();

                      // Replace argument block to ensure members are ordered by identifier
                      // NOTE: About member ordering - If we didn't order members then users could declare multiple structural tuples of the same "type" with different order and break serialization/deserialization.
                      block = new ResolvedBlock( orderedMembers, null );
                      return orderedMembers.Select( member => member.Value
                                                                    .InstanceType( context )
                                                                    .Map( type => (member.Identifier, member.Value, Type: type) ) )
                                           .ToResultArray();
                  } )
                 .Bind( members =>
                  {
                      bool IsTupleOfCorrectType(StructuralTuple otherTuple)
                      {
                          // Check tuples have same number of members
                          var result = members.Length == otherTuple._fields.Count;
                          if (!result) return false;


                          for (var index = 0; index < members.Length; index++)
                          {
                              var (identifier, _, type) = members[index];
                              var (otherIdentifier, _, otherType) = otherTuple._fields[index];
                              result |= identifier.Equals( otherIdentifier ) && type.Equals( otherType );
                          }

                          return result;
                      }

                      // Find the tuple type in context's known structural tuples or generate and add a new one
                      if (!(context.StructuralTuples.FirstOrDefault( IsTupleOfCorrectType ) is { } tupleType))
                      {
                          tupleType = new StructuralTuple( members );
                          context.StructuralTuples.Add( tupleType );
                      }

                      return new Result<IValue>( new StructuralTupleInstance( block, tupleType ) );
                  } );

        public override string SummaryString => $"Tuple<{string.Join( ", ", _fields.Select( f => f.Type ) )}>";

        public override Result<bool> MatchesConstraint(IValue value, Context context) => value.IsInstanceOfType( this, context );

        private sealed class StructuralTupleInstance : Value
        {
            public StructuralTuple TupleType { get; }

            private readonly ResolvedBlock _resolvedBlock;

            public StructuralTupleInstance(ResolvedBlock block, StructuralTuple tupleType)
            {
                TupleType = tupleType;
                _resolvedBlock = block;
            }

            public override string TypeOf => $"Tuple<{string.Join( ", ", TupleType._fields.Select( f => f.Type ) )}>";
            public override Result<IValue> InstanceType(Context context) => TupleType;
            public override Result<IValue> Index(Identifier id, Context context) => _resolvedBlock.Index( id, context );
            public override IReadOnlyList<Identifier> Members => _resolvedBlock.Members;

            public override void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context) => _resolvedBlock.Serialize( resultBuilder, context );

            public override Result<IValue> Deserialize(Func<Instruction> nextValue, Context context) =>
                _resolvedBlock.DeserializeMembers( nextValue, context )
                              .Map( deserializedFields =>
                               {
                                   var newBlock = new ResolvedBlock( deserializedFields
                                                                    .Zip( _resolvedBlock.Members,
                                                                         (value, identifier) => (identifier, value) )
                                                                    .ToArray(), null );
                                   return new StructuralTupleInstance( newBlock, TupleType );
                               } ).Cast<IValue>();
        }
    }
}