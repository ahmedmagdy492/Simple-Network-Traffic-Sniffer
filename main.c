#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>

int tun_alloc(char* dev) {
	int fd, err;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));

	if((fd = open("/dev/net/tun", O_RDWR)) < 0) {
		perror("open");
		return fd;
	}

	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	if(*dev) {
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	if((err = ioctl(fd, TUNSETIFF, (void*)&ifr)) < 0) {
		perror("ioctl");
		close(fd);
		return err;
	}

	strcpy(dev, ifr.ifr_name);

	return fd;
}

void process_packets(const char *buffer, size_t buf_len) {

	if(sizeof(struct ethhdr) > buf_len) {
		printf("packet length is too short\n");
		return;
	}

	const struct ethhdr* eth = (const struct ethhdr*)buffer;
	
	printf("Source MAC: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
           eth->h_source[0], eth->h_source[1], eth->h_source[2],
           eth->h_source[3], eth->h_source[4], eth->h_source[5]);
	printf("Destination MAC: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
           eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
           eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);

	unsigned short h_proto = ntohs(eth->h_proto);
	switch (h_proto) {
        	case 0x0800: // IPv4
            		printf("Protocol: IPv4\n");
            		// Parse IPv4 header and payload here...
	            	break;
        	case 0x86dd: // IPv6
            		printf("Protocol: IPv6\n");
        		// Parse IPv6 header and payload here...
		        break;
	        default:
            		printf("Unknown protocol: 0x%04hx\n", h_proto);
            		break;
	}  
}

int main() {
	char tun_name[IFNAMSIZ];
	// setting the first char to null to prevent the kernel from using the existing string as the device name
	// and make the kernel come up with a name for the deivce
	tun_name[0] = '\0';
	int fd;

	if((fd = tun_alloc(tun_name)) < 0) {
		perror("tun_alloc");
		return 1;
	}

	printf("allocated tun device: %s\n", tun_name);


	while(1) {
		char buf[2048];
		ssize_t nread;

		nread = read(fd, buf, sizeof(buf));
		if(nread < 0) {
			perror("read");
			break;
		}

		process_packets(buf, nread);
	}


	close(fd);

	return 0;
}
