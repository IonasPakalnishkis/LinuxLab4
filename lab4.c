#define FUSE_USE_VERSION 26

#include <sys/types.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct directory
{
	int isFree;
	char path[255];
	char name[255];
};

int dirsfd, startPos = 0;
struct directory dir;

static int getattr_callback(const char *path, struct stat *stbuf) 
{
	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0) 
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}
	lseek(dirsfd, startPos, SEEK_SET);
	while (read(dirsfd, &dir, sizeof(struct directory)) != 0) 
	{
		if ((dir.isFree == 0) && (strcmp(dir.path, path) == 0)) 
		{
			stbuf->st_mode = S_IFDIR | 0777;
			stbuf->st_nlink = 2;
			return 0;
		}
	}
	return -ENOENT;
}

static int mkdir_callback(const char* path, mode_t mode)
{
	lseek(dirsfd, startPos, SEEK_SET);
	while (read(dirsfd, &dir, sizeof(struct directory)) != 0) 
	{
		if (!dir.isFree && strcmp(path, dir.path) == 0)
			return -ENOENT;
	}
	lseek(dirsfd, startPos, SEEK_SET);
	int curpos = 0;
	while (read(dirsfd, &dir, sizeof(struct directory)) != 0) 
	{
		if (dir.isFree) 
		{
			dir.isFree = 0;
			strcpy(dir.path, path);
			int ps = strlen(path) - 1;
			while ((path[ps] != '/') && (ps >= 0))
				ps--;
			if (ps < 0)
				return -ENOENT;
			strcpy(dir.name, path + ps);
			lseek(dirsfd, curpos, SEEK_SET);
			lseek(dirsfd, startPos + curpos * sizeof(struct directory), SEEK_SET);
			write(dirsfd, &dir, sizeof(struct directory));
			return 0;
		}
		curpos++;
	}
	dir.isFree = 0;
	int ps = strlen(path) - 1;
	while ((path[ps] != '/') && (ps >= 0))
		ps--;
	if (ps < 0)
		return -ENOENT;
	strcpy(dir.path, path);
	strcpy(dir.name, path + ps);
	write(dirsfd, &dir, sizeof(struct directory));
		lseek(dirsfd, startPos, SEEK_SET);
	return 0;
}



static struct fuse_operations fuse_example_operations = 
{
	.getattr = getattr_callback,
	.open = open_callback,
	.read = read_callback,
	.readdir = readdir_callback,
	.mkdir = mkdir_callback,
	.rmdir = rmdir_callback,
};

int main(int argc, char *argv[])
{
	dirsfd = open("./directories", O_RDWR | O_CREAT, 0777);
	int ret = fuse_main(argc, argv, &fuse_example_operations, NULL);
	close(dirsfd);
	return ret;
}
