# MemoryScanner

Run as sudo since reading/writing to /proc/pid/mem requires root permissions.

(Currently) only scans integers. Searches only memory addresses from /proc/pid/maps where the address range has the exact perms "rw-p".
Starts from the beginning in /proc/pid/maps and ends with (including) the heap. So therefore the stack is (currently) excluded.
