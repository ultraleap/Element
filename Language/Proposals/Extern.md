Some situations require programmers to reference externally defined data/functionality.
For example, sourcing amplitude from a waveform.
```
struct Waveform(samples:Indexer);
extern MyWaveform:Waveform;
sample = MyWaveform.samples(500);
```

Extern is distinct from intrinsics in several ways:
* Intrinsic's may be any function, type or constraint declaration.
* An intrinsic's source declaration is fully authoritative, i.e. input/output port declared in source are applied normally.
* Intrinsics cannot always be declared fully in source (e.g. variadic intrinsics), the implementation may do extra verification.
* If the source declaration of an intrinsic does not match what the host expects, the behaviour is undefined.
For this reason, intrinsics are generally declared in the Prelude and versioned alongside the host.
* A host is not required to support adding custom intrinsic implementations.

An extern declaration has the following differences:
* Extern declaration must be a function with no inputs and have a defined return type.
The return type is used by the host to determine the boundary interface.
The return type may include any values (even non-serializable) as long as platforms can implement them.
* The source declaration of an extern type is compared with the host's definition.
If they do not match, an error must be produced.
* A host is required to support adding custom extern types.