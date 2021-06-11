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
#define keyLen 5
char dirpath[maxSize];
char key[] = "SISOP";
//melakukan encode/decode
char *AtoZ_code(char src[])
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

void fsLog(char *level, char *cmd,int descLen, const char *desc[])
{
    FILE *f = fopen(FSLogPath, "a");
    time_t now;
	time ( &now );
	struct tm * timeinfo = localtime (&now);
	fprintf(f, "%s::%s::%02d%02d%04d-%02d:%02d:%02d",level,cmd,timeinfo->tm_mday,timeinfo->tm_mon+1,timeinfo->tm_year+1900,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    for(int i = 0; i<descLen; i++)
    {
        fprintf(f,"::%s",desc[i]);
    }
    fprintf(f,"\n");
    fclose(f);
}

char *RX_code(char src[])
{
    char str[maxSize];
    strcpy(str, src);
    int i;
    for (i = 0; i < strlen(str); i++)
    {
        if (str[i] >= 'A' && str[i] <= 'Z')
        {
            if (str[i] < 'N')
                str[i] += 13;
            else
                str[i] -= 13;
        }
        else if (str[i] >= 'a' && str[i] <= 'z')
        {
            if (str[i] < 'n')
                str[i] += 13;
            else
                str[i] -= 13;
        }
    }
    char *res = str;
    return AtoZ_code(res);
}
char *Vigenere(char src[])
{
    char str[maxSize], temp;
    strcpy(str, src);
    int i, keyIndex = 0;
    for (i = 0; i < strlen(str); i++)
    {
        temp = str[i];
        keyIndex %= keyLen;
        if (temp >= 'A' && temp <= 'Z')
        {
            temp -= 'A';
            temp = (temp + (key[keyIndex++] - 'A')) % 26;
            str[i] = temp + 'A';
        }
        else if (temp >= 'a' && temp <= 'z')
        {
            temp -= 'a';
            temp = (temp + (key[keyIndex++] - 'A')) % 26;
            str[i] = temp + 'a';
        }
    }
    printf("%s\n", str);
    char *res = str;
    return AtoZ_code(res);
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

    bool flag, toMirror = 0, toROT = 0;

    // Cek apakah perlu encode/decode (AtoZ_)
    char *strMir, *strRX;
    if (strcmp(path, "/")) // pointer menunjuk pada folder
    {
        strMir = strstr(path, "/AtoZ_");
        if (strMir)
        {
            toMirror = 1;
            strMir++;
        }
        strRX = strstr(path, "/RX_");
        if (strRX)
        {
            toROT = 1;
            strRX++;
        }
    }

    if (strcmp(path, "/") == 0)
    {
        sprintf(fpath, "%s", dirpath);
        // path masih di folder Downloads
    }
    else if (toMirror || toROT)
    {
        char pathOrigin[maxSize] = "";
        char t[maxSize]; // buka directory
        if (toMirror)
        {
            strncpy(pathOrigin, path, strlen(path) - strlen(strMir));
            strcpy(t, strMir);
        }

        else if (toROT)
        {
            strncpy(pathOrigin, path, strlen(path) - strlen(strRX));
            strcpy(t, strRX);
        }

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
                sprintf(pathFolder, "%s%s%s", dirpath, pathOrigin, selectedFile); // Path dari direktori Downloads

                DIR *dp = opendir(pathFolder);
                if (!dp)
                {
                    char *ext;
                    ext = strrchr(selectedFile, '.');

                    char fileName[maxSize] = "";
                    if (ext) // Akses jika berekstensi
                    {
                        // Mengubah nama file jika memiliki extensi tanpa mengubah ext file tersebut
                        strncpy(fileName, selectedFile, strlen(selectedFile) - strlen(ext));
                        if (toMirror)
                        {
                            sprintf(fileName, "%s%s", AtoZ_code(fileName), ext);
                        }
                        else if (toROT)
                        {
                            sprintf(fileName, "%s%s", RX_code(fileName), ext);
                        }
                    }
                    else // akses folder
                    {
                        if (toMirror)
                        {
                            strcpy(fileName, AtoZ_code(selectedFile));
                        }
                        else if (toROT)
                        {
                            strcpy(fileName, RX_code(selectedFile));
                        }
                    }
                    printf("%s\n", fileName);
                    strcat(pathOrigin, fileName); // Semua nama file di lokasi sumber (asli)
                }
                else
                {
                    if (toMirror)
                        strcat(pathOrigin, AtoZ_code(selectedFile));
                    else if (toROT)
                    {
                        strcat(pathOrigin, RX_code(selectedFile));
                    }
                }
            }
            else
            {
                if (toMirror)
                {
                    strcat(pathOrigin, AtoZ_code(selectedFile));
                }
                else if (toROT)
                {
                    strcat(pathOrigin, RX_code(selectedFile));
                }
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
    bool toMirror = strstr(path, "/AtoZ_"), toROT = strstr(path, "/RX_");

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
        else if (toMirror || toROT)
        {
            if (de->d_type & DT_DIR)
            {
                char temp[maxSize];
                strcpy(temp, de->d_name);

                if (toMirror)
                    strcpy(temp, AtoZ_code(temp));
                else if (toROT)
                {
                    strcpy(temp, RX_code(temp));
                }

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

                    if (toMirror)
                        strcpy(fileName, AtoZ_code(fileName));
                    else if (toROT)
                    {
                        strcpy(fileName, RX_code(fileName));
                    }

                    strcat(fileName, ext);
                }
                else
                {
                    if (toMirror)
                        strcpy(fileName, AtoZ_code(de->d_name));
                    else if (toROT)
                    {
                        strcpy(fileName, RX_code(de->d_name));
                    }
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
    char *filepath = strchr(des, '/');
    if (strstr(filepath, "/AtoZ_") || strstr(filepath, "/RX_"))
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
    char *filepath = strchr(path, '/');
    if (strstr(filepath, "/AtoZ_") || strstr(filepath, "/RX_"))
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
