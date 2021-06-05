#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/time.h>
#define maxSize 1024

char dirpath[maxSize];

//melakukan encode/decode
char *endecode(char src[])
{
    char str[maxSize];
    strcpy(str, src);
    int i;
    for (i = 0; i < strlen(str); i++)
    {
        if (str[i] >= 'A' && str[i] <= 'Z')
        {
            str[i] = 'A' + 'Z' - str[i];
        }
        else if (str[i] >= 'a' && str[i] <= 'z')
        {
            str[i] = 'a' + 'z' - str[i];
        }
    }
    char *res = str;
    return res;
}

// Membuat log sesuai format, 1 jika rename dan 2 jika mkdir
void createLog(char old[], char new[], int newFolder)
{
    FILE *file = fopen("encode.log", "a");

    char str[2 * maxSize];
    if (newFolder == 1)
    {
        sprintf(str, "(Rename)%s --> %s", old, new);

        fprintf(file, "%s\n", str);
    }
    else if (newFolder == 2)
    {
        sprintf(str, "(Mkdir)%s", new);
        fprintf(file, "%s\n", str);
    }
    fclose(file);
}

char *find_path(const char *path)
{
    char fpath[2 * maxSize];

    bool flag, toMirror = 0;

    // Cek apakah perlu encode/decode (AtoZ_)
    char *strMir;
    if (strcmp(path, "/")) // pointer menunjuk pada folder
    {
        strMir = strstr(path, "/AtoZ_");
        if (strMir)
        {
            toMirror = 1;
            strMir++;
        }
    }

    if (strcmp(path, "/") == 0)
    {
        sprintf(fpath, "%s", dirpath);
        // path masih di folder Downloads
    }
    else if (toMirror)
    {
        char pathOrigin[maxSize] = "";
        strncpy(pathOrigin, path, strlen(path) - strlen(strMir));
        // isinya folder AtoZ_ pertama

        char t[maxSize];
        strcpy(t, strMir);

        char *selectedFile;
        char *rest = t;

        flag = 0;

        // rekursif untuk melihat isi di dalam folder
        while ((selectedFile = strtok_r(rest, "/", &rest)))
        {
            if (!flag)
            {
                strcat(pathOrigin, selectedFile);
                // printf("pathOrigin/selectedFile %s == %s\n", pathOrigin, selectedFile);
                flag = 1;
                continue;
            }
            // Cek extension
            char checkType[2 * maxSize];
            sprintf(checkType, "%s/%s", pathOrigin, selectedFile);
            strcat(pathOrigin, "/");

            if (strlen(checkType) == strlen(path))
            {
                char pathFolder[2 * maxSize];
                sprintf(pathFolder, "%s%s%s", dirpath, pathOrigin, selectedFile);

                DIR *dp = opendir(pathFolder);
                if (!dp)
                {
                    char *ext;
                    ext = strrchr(selectedFile, '.');

                    char fileName[maxSize] = "";
                    if (ext)
                    {
                        // Mengubah nama file jika memiliki extensi tanpa mengubah ext file tersebut
                        strncpy(fileName, selectedFile, strlen(selectedFile) - strlen(ext));
                        sprintf(fileName, "%s%s", endecode(fileName), ext);
                    }
                    else
                    {
                        strcpy(fileName, endecode(selectedFile));
                    }
                    printf("%s\n", fileName);
                    strcat(pathOrigin, fileName); // Semua nama file di lokasi sumber (asli)
                }
                else
                {
                    strcat(pathOrigin, endecode(selectedFile));
                }
            }
            else
            {
                strcat(pathOrigin, endecode(selectedFile));
            }
        }
        sprintf(fpath, "%s%s", dirpath, pathOrigin);
    }
    else
        sprintf(fpath, "%s%s", dirpath, path);

    char *fpath_to_return = fpath;
    return fpath_to_return; // Langsung return fpath bakal bikin segmented fault
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    printf("\n===Directory '%s'===\n", path);
    char fpath[maxSize];
    bool toMirror = strstr(path, "/AtoZ_");

    strcpy(fpath, find_path(path));

    int res = 0;
    DIR *dp;
    struct dirent *de;

    (void)offset;
    (void)fi;

    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL)
    {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            res = (filler(buf, de->d_name, &st, 0));
        }
        else if (toMirror)
        {
            if (de->d_type & DT_DIR)
            {
                char temp[maxSize];
                strcpy(temp, de->d_name);
                strcpy(temp, endecode(temp));
                res = (filler(buf, temp, &st, 0));
            }
            else
            {
                char *ext;
                ext = strrchr(de->d_name, '.');

                char fileName[maxSize] = "";
                if (ext)
                {
                    strncpy(fileName, de->d_name, strlen(de->d_name) - strlen(ext));
                    strcpy(fileName, endecode(fileName));
                    strcat(fileName, ext);
                }
                else
                {
                    strcpy(fileName, endecode(de->d_name));
                }
                res = (filler(buf, fileName, &st, 0));
            }
        }
        else
            res = (filler(buf, de->d_name, &st, 0));

        if (res != 0)
            break;
    }
    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char fpath[maxSize];
    strcpy(fpath, find_path(path));

    int fd;
    int res;

    (void)fi;
    fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    char fpath[maxSize];
    strcpy(fpath, find_path(path));

    int res;
    res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_rename(const char *src, const char *des)
{
    char *checkEndecode = strchr(des, '/');
    if (strstr(checkEndecode, "/AtoZ_"))
    {
        char oldName[maxSize];
        char newName[maxSize];

        sprintf(oldName, "%s%s", dirpath, src);
        sprintf(newName, "%s%s", dirpath, des);

        printf("Renaming %s to %s in %s\n", src, des, dirpath);
        createLog(oldName, newName, 1);
    }

    char file_src[maxSize];
    char file_des[maxSize];

    strcpy(file_src, find_path(src));
    strcpy(file_des, find_path(des));

    int res;

    res = rename(file_src, file_des);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
    char *checkEndecode = strchr(path, '/');
    if (strstr(checkEndecode, "/AtoZ_"))
    {
        char temp[maxSize];
        sprintf(temp, "%s%s", dirpath, path);

        printf("\nCreating directory %s in %s\n", path, dirpath);
        createLog("", temp, 2);
    }

    char fpath[maxSize];
    strcpy(fpath, find_path(path));
    int res;

    res = mkdir(fpath, mode);
    if (res == -1)
        return -errno;

    return 0;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .read = xmp_read,
    .rename = xmp_rename,
    .mkdir = xmp_mkdir,
};

int main(int argc, char *argv[])
{
    char *username = getenv("USER");

    sprintf(dirpath, "/home/%s/Downloads", username);

    umask(0);

    FILE *file = fopen("encode.log", "a");
    time_t t;
    time(&t);

    fprintf(file, "\nMount at: %s", ctime(&t));
    fclose(file);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
