#include <errno.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define VERIFY_YUBIKEY "Yubico\n"
#define USB_DEVICE_CHARS "0123456789-:."
#define MAX_USB_LEN 16

int debug = 0;

int test_yubikey(char *yubikey) {
	char buf[128];
	int fd;
	int retval;

	if (strspn(yubikey, USB_DEVICE_CHARS) != strlen(yubikey)) {
		return 0;
	}
	if (strlen(yubikey) > MAX_USB_LEN) {
		return 0;
	}

	sprintf(buf, "/sys/bus/usb/devices/%s/manufacturer", yubikey);

	if (debug)
		fprintf(stderr, "opening %s\n", buf);

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return 0;
	
	retval = read(fd, buf, sizeof(buf));
	if (retval < 0) {
		close(fd);
		return 0;
	}
	close(fd);
	buf[retval] = '\0';

	if (debug)
		fprintf(stderr, "read %.*s\n", retval-1, buf);

	if (strncmp(buf, VERIFY_YUBIKEY, sizeof(VERIFY_YUBIKEY)-1) != 0) {
		return 0;
	}

	return 1;
}

typedef void (*callback_t)(char *);

void iterate_yubikeys(callback_t callback) {
	DIR *usbhid;
	DIR *device;
	struct dirent *ent;

	usbhid = opendir("/sys/bus/usb/devices/");
	if (!usbhid) {
		perror("opening usbhid directory");
		exit(1);
	}
	for (ent=readdir(usbhid); ent; ent=readdir(usbhid)) {
		char name[MAX_USB_LEN+1];

		if (ent->d_name[0] == '.')
			continue;
		if (strlen(ent->d_name) > MAX_USB_LEN)
			continue;

		strcpy(name, ent->d_name);
		if (test_yubikey(name)) {
			char buf[128] = "/sys/bus/usb/devices/";
			strcat(buf, name);

			if (debug)
				fprintf(stderr, "Found %s, iterating over device\n", name);

			device = opendir(buf);
			if (!device) {
				perror("opening device directory");
				exit(1);
			}

			for (ent=readdir(device); ent; ent=readdir(device)) {
				char yubikey[MAX_USB_LEN+1];

				if (ent->d_name[0] == '.')
					continue;
				if (strlen(ent->d_name) > MAX_USB_LEN)
					continue;
				if (strspn(ent->d_name, USB_DEVICE_CHARS) != strlen(ent->d_name))
					continue;

				strcpy(yubikey, ent->d_name);

				if (debug)
					fprintf(stderr, "Found %s\n", yubikey);

				callback(yubikey);
			}
		}
	}
}

char bound_status(char *yubikey) {
	int retval;
	struct stat statbuf;
	char buf[128] = "/sys/bus/usb/drivers/usbhid/";

	strcat(buf, yubikey);
	retval = lstat(buf, &statbuf);
	if (retval < 0 && errno == ENOENT)
		return 0;
	else if (retval == 0) {
		return 1;
	} else {
		perror("lstat in usbhid");
		exit(1);
	}
}


void callback_on(char *yubikey) {
	int retval, fd;

	fprintf(stderr, "binding Yubikey to usbhid driver at %s\n", yubikey);

	if (bound_status(yubikey)) {
		fprintf(stderr, "Yubikey already bound to usbhid, skipping\n");
		return;
	}

	fd = open("/sys/bus/usb/drivers/usbhid/bind", O_WRONLY);
	if (fd < 0) {
		perror("opening unbind");
		return;
	}
	retval = write(fd, yubikey, strlen(yubikey));
	if (retval < 0) {
		perror("writing bind");
		return;
	}
	close(fd);
}

void callback_off(char *yubikey) {
	int retval, fd;

	fprintf(stderr, "Unbinding Yubikey from usbhid driver at %s\n", yubikey);

	if (!bound_status(yubikey)) {
		fprintf(stderr, "Yubikey not bound to usbhid, skipping\n");
		return;
	}

	fd = open("/sys/bus/usb/drivers/usbhid/unbind", O_WRONLY);
	if (fd < 0) {
		perror("opening unbind");
		exit(1);
	}
	retval = write(fd, yubikey, strlen(yubikey));
	if (retval < 0) {
		perror("writing unbind");
		exit(1);
	}
	close(fd);
}

void callback_list(char *yubikey) {
	fprintf(stderr, "Found Yubikey at %s\n", yubikey);
	if (bound_status(yubikey))
		fprintf(stderr, "Yubikey is bound to usbhid\n");
	else
		fprintf(stderr, "Yubikey is not bound to usbhid\n");
}

int main(int argc, char *argv[], char *envp[]) {

	callback_t callback;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s {on|off|list|debug}\n", argv[0]);
		exit(1);
	}

	if (!strcmp(argv[1], "on")) {
		callback = &callback_on;
	} else if  (!strcmp(argv[1], "off")) {
		callback = &callback_off;
	} else if (!strcmp(argv[1], "list")) {
		callback = &callback_list;
	} else if (!strcmp(argv[1], "debug")) {
		debug = 1;
		callback = &callback_list;
	} else {
		fprintf(stderr, "Usage: %s {on|off|list|debug}\n", argv[0]);
		exit(1);
	}

	iterate_yubikeys(callback);
	exit(0);
}
