#include <linux/watchdog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

const char stopchar = 'V';
const char patchar = 'p'; // can be anything but the stop char

void usage(char* name, FILE *out) {
  fprintf(out, "\n");
  fprintf(out, "%s [start|stop|pat]\n", name);
  fprintf(out, "    -d device : Device file to open (default: /dev/watchdog)\n");
  fprintf(out, "    -t timeout: Reboot after this many seconds with no pat (default: 30)\n");
  fprintf(out, "    -b        : Report whether last reboot was triggered by watchdog, then exit.\n");
  fprintf(out, "\n");
}

int pat(int fd) {
  int ret;
  ret = write(fd, &patchar, 1);
  if(ret != 1) {
    return -1;
  }
  return 0;
}

int stop(int fd, int doMagicClose) {
  int ret;
  if(doMagicClose) {
    ret = write(fd, &stopchar, 1);
    if(ret != 1) {
      fprintf(stderr, "Could not write magic close character! Watchdog might still be active!\n");
      close(fd);
      return -1;
    }
  }
  ret = close(fd);
  if(ret < 0) {
    fprintf(stderr, "Could not close watchdog device! Watchdog might still be active!\n");
    return 1;
  }
  return 0;
}

int reportBootstatus() {
  struct watchdog_info info;
  int ret;
  int fd;
  int boot_status;
  
  ret = ioctl(fd, WDIOC_GETSUPPORT, &info);
  if(ret != 0) {
    fprintf(stderr, "Checking which features are supported by watchdog failed: %s\n", strerror(errno));
    return -1;
  }

  if(!(info.options & WDIOF_CARDRESET)) {
    fprintf(stderr, "Watchdog does not support checking boot status.\n");
    return -1;
  }
  
  ret = ioctl(fd, WDIOC_GETBOOTSTATUS, &boot_status);
  if(ret != 0) {
    fprintf(stderr, "Getting boot status failed: %s\n", strerror(errno));
    return -1;
  }  
  
  if(boot_status & WDIOF_CARDRESET) {
    return 1;
  }
  return 0;
}

int check_support(int fd, int setTimeout) {
  struct watchdog_info info;
  unsigned int timeout;
  int ret;
  
  ret = ioctl(fd, WDIOC_GETSUPPORT, &info);
  if(ret != 0) {
    fprintf(stderr, "Checking which features are supported by watchdog failed: %s\n", strerror(errno));
    return -1;
  }
  if(!(info.options & WDIOF_MAGICCLOSE)) {
    fprintf(stderr, "Watchdog does not support magic close.\n");
    return -2;
  }
  
  if(!(info.options & WDIOF_SETTIMEOUT)) {
    
    ret = ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
    if(ret != 0) {
      fprintf(stderr, "Could neither get nor set watchdog timeout: %s\n", strerror(errno));
      return -1;
    }
    if(setTimeout != timeout) {
      fprintf(stderr, "Could not set watchdog timeout and timeout is currently set to %d.", timeout);
      return -1;
    }
  }
  return 0;
}

int setTimeout(int fd, int timeout) {
  int ret;

  if(timeout <=0) {
    fprintf(stderr, "Timeout must be more than 0\n");
    return -1;
  }

  ret = ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
  if(ret != 0) {
    fprintf(stderr, "Setting timeout failed\n");
    return -1;
  }
  
  return -1;
}


int main(int argc, char** argv) {
  int c;
  int ret;
  int magicClose;
  char defaultDevPath[] = "/etc/watchdog";
  char* devPath = NULL;
  char* timeoutStr = NULL;
  int timeout = 30; // default timeout
  char* cmd = NULL;
  
  while((c = getopt(argc, argv, "bd:t:")) != -1) {
    switch(c) {
    case 'b':
      ret = reportBootstatus();
      if(ret < 0) {
        return 1;
      }
      if(ret == 1) {
        printf("Last reboot triggered by watchdog: no\n");
      } else {
        printf("Last reboot triggered by watchdog: yes.\n");
      }
      return 0;
      break;
    case 'd':
      devPath = optarg;
      break;
    case 't':
      timeoutStr = optarg;
      break;
    default:
      fprintf(stderr, "Unsupported option '%c'.\n", c);
      usage(argv[0], stderr);
      return 1;
    }
  }
  
  if(optind < argc) {
    cmd = argv[optind];
  }
  
  if(!cmd) {
    usage(argv[0], stderr);
    return 1;
  }
  
  if(!devPath) {
    devPath = defaultDevPath;
  }
  
  if(timeoutStr) {
    timeout = atoi(timeoutStr);
    if((timeout < 1) || (timeout > 86400)) {
      fprintf(stderr, "Timeout must be between 1 and 86400\n");
      return 1;
    }
    printf("Setting %d\n", timeout);
  }
  
  int fd = open(devPath, O_WRONLY);
  if(fd < 0) {
    fprintf(stderr, "Could not open device %s\n", devPath);
    return 1;
  }
  
  ret = check_support(fd, timeout);
  if(ret < 0) {
    if(ret < -1) { // if it didn't even support magic close
      stop(fd, 0);
    } else {
      stop(fd, 1);
    }
    return 1;
  }
  
  if(timeoutStr) {
    if(setTimeout(fd, timeout) < 0) {
      return -1;
    }
  }

  if(strcmp(cmd, "start") == 0) {
    pat(fd); // since we already opened it
  } else if(strcmp(cmd, "stop") == 0) {
    ret = stop(fd, 1);
    if(ret < 0) {
      return 1;
    }
    return 0;  
  } else if(strcmp(cmd, "pat") == 0) {
    pat(fd);    
  } else {
    usage(argv[0], stderr);
    return 1;    
  }
  
  ret = close(fd);
  if(ret < 0) {
    fprintf(stderr, "Failed to close watchdog device\n.");
    return 1;
  }
  return 0;
}
