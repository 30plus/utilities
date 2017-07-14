#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <ctype.h>

#include <sys/stat.h>
#include <libgen.h>

#define countof(X) (sizeof(X) / sizeof(X[0]))

#define TRUE  1
#define FALSE 0

int fexists(const char* path)
{
	char buf[PATH_MAX];
	struct stat info;

	realpath(path, &buf[0]);
  
	if (lstat(buf, &info) != 0) {
		if (errno == ENOENT || errno == EACCES) {
			return FALSE;
		}
	}

	return S_ISREG(info.st_mode) ? TRUE : FALSE;
}

int dexists(const char* path)
{
	char buf[PATH_MAX];
	struct stat info;

	realpath(path, &buf[0]);
  
	if (lstat(buf, &info) != 0) {
		if (errno == ENOENT || errno == EACCES)
			return FALSE;
	}

	return S_ISDIR(info.st_mode) ? TRUE : FALSE;
}

size_t fsize(FILE* fp)
{
	fseek(fp, 0L, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return size;
}

char* rstrip(char* s)
{
	size_t len = strlen(s);

	for (int i = len - 1; i >= 0; i--) {
		if (!isspace(s[i]))
			break;
		s[i] = '\0';
	}
	return s;
}

void xfree(void* lp) {
	if (lp != NULL)
		free(lp);
}

int strdump(const char* path, const char* value, int retry)
{
	FILE* fp = fopen(path, "w");
	size_t size = strlen(value);

	if (fp != NULL) {
		size_t nbytes = 0;

		do {
			nbytes += fwrite(value, 1, size - nbytes, fp);
			if (errno != 0)
				break;

		} while (retry == TRUE && nbytes < size);
		fclose(fp);
	}
	return errno;
}

int strload(const char* path, char** pbuf, int retry)
{
  FILE* fp = fopen(path, "r");

  if (fp != NULL) {
    size_t size = fsize(fp);
    size_t nbytes = 0;

    char* buf = malloc(size + 1);
    buf[size] = '\0';

    do {
      nbytes += fread(buf + nbytes, 1, size - nbytes, fp);

      if (errno != 0) {
	free(buf);
	break;
      }
    } while (retry == TRUE && nbytes < size);
    
    *pbuf = buf;
    fclose(fp);
  }
  return errno;
}

const char* newid_path = "/sys/bus/pci/drivers/vfio-pci/new_id";
const char* remid_path = "/sys/bus/pci/drivers/vfio-pci/remove_id";
const char* probe_path = "/sys/bus/pci/drivers_probe";
const char* vfctl_path = "/etc/vfio/vfioctl.lst";

const char* driver_unbind_fmt = "/sys/bus/pci/devices/%s/driver/unbind";
const char* vfiodr_unbind_fmt = "/sys/bus/pci/drivers/vfio-pci/unbind";

const char* device_fmt = "/sys/bus/pci/devices/%s/%s";
char* vfctl_allowed[64];

int vfio_allowed(const char* dev)
{
	for (int i = 0; i < countof(vfctl_allowed) && vfctl_allowed[i] != NULL; i++) {
		if (strcmp(dev, vfctl_allowed[i]) == 0)
			return TRUE;
	}
	return FALSE;
}

int vfio_bind(const char* dev)
{
  char* vendor = NULL;
  char* device = NULL;

  char buf[BUFSIZ];

  if (vfio_allowed(dev) == FALSE) {
    return EPERM;
  }

  snprintf(buf, sizeof(buf), device_fmt, dev, "vendor");
  if (strload(buf, &vendor, FALSE) != 0)
    goto _err;

  snprintf(buf, sizeof(buf), device_fmt, dev, "device");
  if (strload(buf, &device, FALSE) != 0)
    goto _err;

  snprintf(buf, sizeof(buf), device_fmt, dev, "driver");
  if (dexists(buf) == TRUE) {
    snprintf(buf, sizeof(buf), driver_unbind_fmt, dev);
    if (strdump(buf, dev, TRUE) != 0)
      goto _err;
  }

  snprintf(buf, sizeof(buf), "%s %s\n", rstrip(vendor), rstrip(device));
  if (strdump(newid_path, buf, TRUE) != 0)
    goto _err;

_err:
  xfree(device);
  xfree(vendor);
  return errno;
}

int vfio_unbind(const char* dev)
{
  char* vendor = NULL;
  char* device = NULL;

  char buf[BUFSIZ];

  if (vfio_allowed(dev) == FALSE) {
    return EPERM;
  }

  snprintf(buf, sizeof(buf), device_fmt, dev, "vendor");
  if (strload(buf, &vendor, FALSE) != 0)
    goto _err;

  snprintf(buf, sizeof(buf), device_fmt, dev, "device");
  if (strload(buf, &device, FALSE) != 0)
    goto _err;

  snprintf(buf, sizeof(buf), device_fmt, dev, "driver");
  if (dexists(buf)) {
    snprintf(buf, sizeof(buf), vfiodr_unbind_fmt, dev);
    if (strdump(buf, dev, TRUE) != 0)
      goto _err;
  }

  snprintf(buf, sizeof(buf), "%s %s\n", rstrip(vendor), rstrip(device));
  if (strdump(remid_path, buf, TRUE) != 0)
    goto _err;

  if (strdump(probe_path, dev, TRUE) != 0)
    goto _err;

 _err:
  xfree(device);
  xfree(vendor);
  return errno;
}

void do_bind(char** argv, int beg, int end)
{
	for (int i = beg; i < end; i++)
		printf("Binding [%s] -> %s\n", argv[i], strerror(vfio_bind(argv[i])));
}

void do_unbind(char** argv, int beg, int end)
{
	for (int i = beg; i < end; i++)
		printf("Unbinding [%s] -> %s\n", argv[i], strerror(vfio_unbind(argv[i])));
}

int load_vfctl(char** vfctl)
{
	if (strload(vfctl_path, vfctl, TRUE) != 0)
		goto _err;
  
	vfctl_allowed[0] = strtok(*vfctl, " \r\n");

	for (int i = 1; i < countof(vfctl_allowed) && vfctl_allowed[i - 1] != NULL; i++) {
		vfctl_allowed[i] = strtok(NULL, " \r\n");
	}

_err:
	return errno;
}

void help() {
	puts("Usage:");
	puts("  vfioctl command device-ids");
	puts("  command :: bind, unbind");
}

int main(int argc, char *argv[])
{
	setuid(geteuid());
	setgid(getegid());

	if (argc > 1) {
		char* base = basename(argv[0]);
		char* vfctl = NULL;

		if (load_vfctl(&vfctl) != 0) {
			puts("Failed loading vfioctl.lst");
			return EXIT_FAILURE;
		}

		if (strcmp(base, "vfio-bind") == 0)
			do_bind(argv, 1, argc);
		else if (strcmp(base, "vfio-unbind") == 0)
			do_unbind(argv, 1, argc);
		else {
			if (strcmp(argv[1], "bind") == 0)
				do_bind(argv, 2, argc);
			else if (strcmp(argv[1], "unbind") == 0)
				do_unbind(argv, 2, argc);
			else
				help();
		}
		xfree(vfctl);
	} else {
		help();
	}
	return EXIT_SUCCESS;
}
