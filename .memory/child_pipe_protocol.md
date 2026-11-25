# Parent-child pipe protocol

Apply each line when changing the pipe communication or field transfer schema between the frequency child processes and the parent.

## User-given

- freq_fields_xfer Child communication schema protocol: must not do any branching in the loop that can create parent and child deviations; Conditionals must only happen based on the fields[i].cond to guarantee parent and child parity.  `.cond` Values must always be defined statically, never conditionally, because the child and parent processes execute in different memory contexts that may deviate over time. 
	- Whenever modifying the pipe communication mechanism, perform extra thinking to guarantee parity between parent and child according to these requirements and other considerations you might see or think of. 
