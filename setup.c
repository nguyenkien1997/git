#include "cache.h"

char *prefix_path(const char *prefix, int len, char *path)
{
	char *orig = path;
	for (;;) {
		char c;
		if (*path != '.')
			break;
		c = path[1];
		/* "." */
		if (!c) {
			path++;
			break;
		}
		/* "./" */
		if (c == '/') {
			path += 2;
			continue;
		}
		if (c != '.')
			break;
		c = path[2];
		if (!c)
			path += 2;
		else if (c == '/')
			path += 3;
		else
			break;
		/* ".." and "../" */
		/* Remove last component of the prefix */
		do {
			if (!len)
				die("'%s' is outside repository", orig);
			len--;
		} while (len && prefix[len-1] != '/');
		continue;
	}
	if (len) {
		int speclen = strlen(path);
		char *n = xmalloc(speclen + len + 1);
	
		memcpy(n, prefix, len);
		memcpy(n + len, path, speclen+1);
		path = n;
	}
	return path;
}

const char **get_pathspec(const char *prefix, char **pathspec)
{
	char *entry = *pathspec;
	char **p;
	int prefixlen;

	if (!prefix && !entry)
		return NULL;

	if (!entry) {
		static const char *spec[2];
		spec[0] = prefix;
		spec[1] = NULL;
		return spec;
	}

	/* Otherwise we have to re-write the entries.. */
	p = pathspec;
	prefixlen = prefix ? strlen(prefix) : 0;
	do {
		*p = prefix_path(prefix, prefixlen, entry);
	} while ((entry = *++p) != NULL);
	return (const char **) pathspec;
}

const char *setup_git_directory(void)
{
	static char cwd[PATH_MAX+1];
	int len, offset;

	/*
	 * If GIT_DIR is set explicitly, we're not going
	 * to do any discovery
	 */
	if (gitenv(GIT_DIR_ENVIRONMENT))
		return NULL;

	if (!getcwd(cwd, sizeof(cwd)) || cwd[0] != '/')
		die("Unable to read current working directory");

	offset = len = strlen(cwd);
	for (;;) {
		/*
		 * We always want to see a .git/refs/ subdirectory
		 */
		if (!access(".git/refs/", X_OK)) {
			/*
			 * Then we need either a GIT_OBJECT_DIRECTORY define
			 * or a .git/objects/ directory
			 */
			if (gitenv(DB_ENVIRONMENT) || !access(".git/objects/", X_OK))
				break;
		}
		chdir("..");
		do {
			if (!offset)
				die("Not a git repository");
		} while (cwd[--offset] != '/');
	}

	if (offset == len)
		return NULL;

	/* Make "offset" point to past the '/', and add a '/' at the end */
	offset++;
	cwd[len++] = '/';
	cwd[len] = 0;
	return cwd + offset;
}
