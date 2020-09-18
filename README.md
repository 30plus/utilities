# utilities
Some Utilities

## pidns
Query for pid mapping inside a container, by leaking a little data.
Only works on Linux 3.10.

Currently supports:
1. query for host pid via pid inside container
	./sample 1 <PID>

2. query for pid inside container via host pid
	./sample 2 <PID>

3. query for corresponding container id of host pid
	./sample 3 <PID> <anything but 1>

4. query for corresponding pod id of host pid
	./sample 3 <PID> 1
