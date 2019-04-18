## Coding challenge - Count field name occurrence

### Original problem description

Please write a program in C that reads and analyzes the content of a file. The file in question contains any number of well-formed
HTTP headers (rfc 2616) and their values in canonical form. Your program needs to read this file and determine the number of times
a set of headers (chosen at compile time) occurs in the file.

For example, if your program tracked the 'Connection', 'Accept', and 'Content-Length' headers, and is run against the following file:

<file start>
Content-Length: 10
User-Agent: Test
Content-Length: 14
Accept: comedy
Content-Length: 100
Content-Encoding: gzip
Connection: close
User-Agent: Test
Accept: flash
User-Agent: Test1
Content-Length: 20
User-Agent: Test2
User-Agent: Test3
Accept: gzip
<file end>

it should indicate that Content-length was seen 4 times, Accept 3 times, etc. As a matter of practice, we recommend that
your program try to match all the headers that a browser might be interested in.

The program does not need to do anything with the header value. Also, when thinking about this solution, please optimize for
CPU, meaning that it's okay to take extra memory if you can find a good algorithm to reduce the processing.

### Revised problem description

The above is very vague and overly confusing.

Scan a text file for lines of text matching the following expression: ^[field-name]:.*$
and then count occurences of each field-name within the file.

Field-name is specified by rfc822; and is characters of ascii 33-126 except for colon.

Allow the fields desired to be processed to be specified when the program is run.

### Initial considerations

Generally to solve a problem such as this it makes the most sense to simple use a red-black tree with a simple
string hashing ( numerical hashing ) algorithm in order to store the counts of the field names.

That said, the last line of the original problem description says to "optimize for CPU and use extra memory".
This seems to imply that what is being looked for is a knowledge of a trie, doing a lookup when each character
is used; a 1->[character list] mapping, instead of a 1->2 binary tree structure using a numerical hash value.
I don't think such a structure is optimal as hash number computation of a tree is extremely fast and binary trees
such as self balancing trees ( red black trees being an example ) are optimal for that sort of lookup.

Another possible option would be to use perfect hashing to take in all possible known header field names and then
use those to point directly into a range of memory capable of handling counts for all possible keys. This would be the
most optimal potentially computation wise as the algorithm for numeric hashing can be minimalized based on the key
space ( limited number of field names that are in actual practical use ). Given a need for the speed difference between
this and a classic hashmap method using a general hashing algorithm, I don't think it is worth going down this route.

If the field names present within the file were given from the start, that would make perfect hashing an optimal
solution. They are not.

Conclusion: Most straightforward sane method of doing this is to use a numeric hashing of field names, and store
counts in a self balancing red-black tree. This said, there is a potential optimization possible here. If you consider
the red-black tree containing the counts to have only "valid paths" leading to counts that you "want to store", you
could skip navigation of the tree once you reach a subtree where no keys that you wish to count are possible to
navigate to. If the list of keys being counted is low, this optimization would save a bunch of memory lookups for
field names that are being ignored. To do this would require tweaking the nodes in the red-black tree implementation
to be able to operate on all the nodes in it leading to the successful tracked values. It would be a fun optimization.

Another possible method would be to use standard numeric hashing and bucketing into a semi-large region of memory.
In this fashion one can do a direct lookup of the count using just a single memory lookup to the node, and then
matching of the full field-name to ensure one is at the correct bucket. Depending on the number of different field
names within the file this will be more optimal / take less memory lookups compared to a tree approach.

This would only, though, be optimal if the file is large enough to warrant the time needed to allocate/reserve the
section of memory. The memory needs to be wiped in that region also. Certain memory allocation methods on Linux
can return already zeroed sections of memory in certain conditions, but it should not be relied on. That could be
mitigated by leaving the program running as service and only zeroing out counts for the field names being looked at
on each run.

Since using a direct lookup seems optimal computation wise I will go with that method.

# Conclusion of implementation method

Use a highly performant numeric hash ( with proper mixing ) to generic a bucket lookup ID with which to store counts
of fields.
