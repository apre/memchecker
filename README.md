# memchecker
A simple c/c++ memory checker

Summary
-------
This module reimplements the c++ new and delete operator
 to get predictable garbage valies after new and delete.
 
  A list of allocated chunks is also maintained, and assert will
  fire-up if trying to deallocate invalid (ie non allocated) blocks.
 
  Also assert fires in case of invalid cross alloc/free (a c++ new followed by a
  C dfree() call).
  Allocation and deallocation are thread-safe (internally protected by mutex).
 
  Main ideas are taken from the awesome book "writing solid code" (ISBN 978-1556155512), and then enhanced
  to handle both C and C++ allocs.


License:
--------
WTFPL v2

