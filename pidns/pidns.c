#include <linux/init.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/compat.h>
#include <linux/ioctl.h>
#include <linux/cgroup.h>

#define PIDNS_CMD_TYPE		'v'
#define CPUSET_SUBSYS_ID	6
#define CGROUP_PATH_MAX		256
//kubepods.slice/kubepods-besteffort.slice/kubepods-besteffort-podd8f61354_2394_410f_870f_c4e14829afac.slice/docker-c6aa250b3667760eb98341b4b7f89d272689621bd9a81dd0cb18ecbf8c0d7d5d.scope                                                                     

/* container.pid => init_ns.pid */
#define PIDNS_CMD_OUT	_IOWR(PIDNS_CMD_TYPE, 1, int32_t *)
/* container.pid <= init_ns.pid */
#define PIDNS_CMD_IN	_IOWR(PIDNS_CMD_TYPE, 2, int32_t *)
/* init_ns.pid => scope */
#define PIDNS_CMD_NS	_IOWR(PIDNS_CMD_TYPE, 3, uint64_t *)

static struct proc_dir_entry *pidns_ent;

static ssize_t pidns_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	char *msg = "Reserved by HIKVision\n";
	int len = strlen(msg);

	if (*ppos > 0 || count < len)
		return 0;

	if (copy_to_user(ubuf, msg, len))
		return -EFAULT;
	*ppos = len;
	return len;
}

static ssize_t pidns_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	*ppos = count;
	return count;
}

static long pidns_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	pid_t pid = 0;
	struct pid *pid_ptr = NULL;
	struct task_struct *task = NULL;
	char ns[CGROUP_PATH_MAX] = {'\0'};

	switch (cmd) {
	case PIDNS_CMD_OUT:
		retval = copy_from_user(&pid, (int32_t *)arg, sizeof(pid_t));
		if (retval)
			return -EFAULT;

		pid_ptr = find_vpid(pid);
		if (pid_ptr)
			pid = pid_ptr->numbers[0].nr;
		else
			pid = 0;

		retval = copy_to_user((int32_t *)arg, &pid, sizeof(pid_t));
		if (retval)
			return -EFAULT;
		break;
	case PIDNS_CMD_IN:
		retval = copy_from_user(&pid, (int32_t *)arg, sizeof(pid_t));
		if (retval)
			return -EFAULT;

		pid_ptr = find_pid_ns(pid, &init_pid_ns);
		if (pid_ptr)
			pid = pid_ptr->numbers[pid_ptr->level].nr;
		else
			pid = 0;

		retval = copy_to_user((int32_t *)arg, &pid, sizeof(pid_t));
		if (retval)
			return -EFAULT;
		break;
	case PIDNS_CMD_NS:
		retval = copy_from_user(&pid, (int32_t *)arg, sizeof(pid_t));
		if (retval)
			return -EFAULT;

		pid_ptr = find_pid_ns(pid, &init_pid_ns);
		task = pid_task(pid_ptr, PIDTYPE_PID);
		if (task) {
			cgroup_path(task_cgroup(task, CPUSET_SUBSYS_ID), ns, CGROUP_PATH_MAX);
		}
		retval = strlen(ns);
		retval = copy_to_user((int32_t *)arg, &ns, retval);
		if (retval)
			return -EFAULT;
		break;
	}
	return retval;
}

static struct file_operations pidns_ops = {
	.owner = THIS_MODULE,
	.read = pidns_read,
	.write = pidns_write,
	.unlocked_ioctl = pidns_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = pidns_ioctl,
#endif
};

static int pidns_init(void)
{
	pidns_ent = proc_create("pidns", 0600, NULL, &pidns_ops);
	return 0;
}

static void pidns_exit(void)
{
	proc_remove(pidns_ent);
}

module_init(pidns_init);
module_exit(pidns_exit);
MODULE_DESCRIPTION("Pid mapper");
MODULE_LICENSE("GPL");
