// ipip.c from https://www.ipip.net/download.html
//
// 感谢 北京天特信科技有限公司
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int debug;

typedef unsigned char byte;
typedef unsigned int uint;

#define B2IL(b) (((b)[0] & 0xFF) | (((b)[1] << 8) & 0xFF00) | (((b)[2] << 16) & 0xFF0000) | (((b)[3] << 24) & 0xFF000000))
#define B2IU(b) (((b)[3] & 0xFF) | (((b)[2] << 8) & 0xFF00) | (((b)[1] << 16) & 0xFF0000) | (((b)[0] << 24) & 0xFF000000))

struct {
	byte *data;
	byte *index;
	uint *flag;
	uint offset;
} ipip;

int destroy()
{
	if (!ipip.offset) {
		return 0;
	}
	free(ipip.flag);
	free(ipip.index);
	free(ipip.data);
	ipip.offset = 0;
	return 0;
}

int init(const char *ipdb)
{
	if (ipip.offset)
		return 0;
	FILE *file = fopen(ipdb, "rb");
	if (file == NULL)
		return 0;
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	ipip.data = (byte *) malloc(size * sizeof(byte));
	if (ipip.data == NULL)
		return 0;
	size_t r = fread(ipip.data, sizeof(byte), (size_t) size, file);

	if (r == 0)
		return 0;

	fclose(file);

	uint length = B2IU(ipip.data);

	if (debug)
		printf("file len=%ld, index_len=%d\n", size, length);

	ipip.offset = length;
	ipip.index = ipip.data + 4;
	ipip.flag = (uint *) ipip.index;

	return 1;
}

int find(const char *ip, char *result, int len)
{
	uint ips[4];
	int num = sscanf(ip, "%d.%d.%d.%d", &ips[0], &ips[1], &ips[2], &ips[3]);
	result[0] = 0;
	if (num == 4) {
		uint ip_prefix_value = ips[0];
		uint ip2long_value = B2IU(ips);
		uint start = ipip.flag[ip_prefix_value];
		uint max_comp_len = ipip.offset - 1028;
		uint index_offset = 0;
		uint index_length = 0;
		for (start = start * 8 + 1024; start < max_comp_len; start += 8) {
			if (B2IU(ipip.index + start) >= ip2long_value) {
				index_offset = B2IL(ipip.index + start + 4) & 0x00FFFFFF;
				index_length = ipip.index[start + 7];
				break;
			}
		}
		if (index_length > len - 1)
			index_length = len - 1;
		memcpy(result, ipip.data + ipip.offset + index_offset - 1024, index_length);
		result[index_length] = '\0';
	}
	return 0;
}
