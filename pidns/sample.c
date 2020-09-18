#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define PID_LEN		32
#define PIDNS_CMD_TYPE	'v'
#define PIDNS_CMD_OUT	_IOWR(PIDNS_CMD_TYPE, 1, int32_t *)
#define PIDNS_CMD_IN	_IOWR(PIDNS_CMD_TYPE, 2, int32_t *)
#define PIDNS_CMD_NS	_IOWR(PIDNS_CMD_TYPE, 3, long long *)

union pidns_u {
	pid_t pid;
#define SCOPE_LEN	256
	char ns[SCOPE_LEN];
};

int scope_from_path(char *path, char **container, char **pod)
{
	int i, j;
	int retval = 0;
	char *ptr = NULL, *idx = NULL;

	if (container)
		*container = NULL;
	if (pod)
		*pod = NULL;

	/* extract container ID */
	ptr = strrchr(path, '/');
	if (ptr == NULL || strlen(ptr) < 64)
		return retval;
	ptr++[0] = '\0';
	if (idx = strchr(ptr, '-'))
		ptr = idx;
	if (ptr[0] == '/' || ptr[0] == '-')
		ptr++;
	if (idx = strchr(ptr, '.'))
		idx[0] = '\0';
	if (container)
		*container = ptr;
	retval++;

	/* extract pod ID */
	if (pod) {
		ptr = strrchr(path, '/');
		if (ptr == NULL || strlen(ptr) < 40)
			return retval;
		ptr++[0] = '\0';

		while (idx = strstr(ptr, "pod"))
			ptr = idx + 3;
		if (idx = strchr(ptr, '.'))
			idx[0] = '\0';
		for (i = 0, j = 0; i < strlen(ptr); i++) {
			ptr[j] = ptr[i];
			if (ptr[i] != '_' && ptr[i] != '-')
				j++;
		}
		ptr[j] = '\0';
		*pod = ptr;
		retval++;
	}
	return retval;
}

/*
 * ./sample 1 <pid in pod>
 * ./sample 2 <pid on host>
 * ./sample 3 <pid on host> <scope type>
 * 		scope_type: 1 for pod, others for container
 */
int main(int argc, char *argv[])
{
	int ret, cmd = 0, scope_type = 0;
	char *scope_ptr = NULL;
	pid_t pid;
	union pidns_u data;
	int fd = open("/proc/pidns", O_RDWR);
	if (fd == -1 || argc < 2)
		return EXIT_FAILURE;

	memset(&data, 0, sizeof(union pidns_u));
	cmd = atoi(argv[1]);
	data.pid = atoi(argv[2]);

	switch (cmd) {
	case 1:
		ret = ioctl(fd, PIDNS_CMD_OUT, &data);
		if (ret == 0)
			printf("%s => %d\n", argv[2], data.pid);
		else
			perror("ioctl failed");
		break;
	case 2:
		ret = ioctl(fd, PIDNS_CMD_IN, &data);
		if (ret == 0)
			printf("%d => %s\n", data.pid, argv[2]);
		else
			perror("ioctl failed");
		break;
	case 3:
		if (argc != 4)
			break;
		scope_type = atoi(argv[3]);
		ret = ioctl(fd, PIDNS_CMD_NS, &data);

		if (scope_type == 1)
			scope_from_path(data.ns, NULL, &scope_ptr);
		else
			scope_from_path(data.ns, &scope_ptr, NULL);

		if (scope_ptr == NULL)
			printf("%s not in a container.\n", argv[2]);
		else
			printf("namespace(%s): %s\n", argv[2], scope_ptr);
		break;
	default:
		printf("Command %d not supported.\n", cmd);
	}

	close(fd);
	return EXIT_SUCCESS;
}
