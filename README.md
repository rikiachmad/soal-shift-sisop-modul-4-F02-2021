# soal-shift-sisop-modul-4-F02-2021
# Nomor 1
Di suatu jurusan, terdapat admin lab baru yang super duper gabut, ia bernama Sin. Sin baru menjadi admin di lab tersebut selama 1 bulan. Selama sebulan tersebut ia bertemu orang-orang hebat di lab tersebut, salah satunya yaitu Sei. Sei dan Sin akhirnya berteman baik. Karena belakangan ini sedang ramai tentang kasus keamanan data, mereka berniat membuat filesystem dengan metode encode yang mutakhir. Berikut adalah filesystem rancangan Sin dan Sei :

> NOTE : 
Semua file yang berada pada direktori harus ter-encode menggunakan Atbash cipher(mirror).
Misalkan terdapat file bernama kucinglucu123.jpg pada direktori DATA_PENTING
“AtoZ_folder/DATA_PENTING/kucinglucu123.jpg” → “AtoZ_folder/WZGZ_KVMGRMT/pfxrmtofxf123.jpg”
Note : filesystem berfungsi normal layaknya linux pada umumnya, Mount source (root) filesystem adalah directory /home/[USER]/Downloads, dalam penamaan file ‘/’ diabaikan, dan ekstensi tidak perlu di-encode.
Referensi : https://www.dcode.fr/atbash-cipher

<ol type="a">
  <li>Jika sebuah direktori dibuat dengan awalan “AtoZ_”, maka direktori tersebut akan menjadi direktori ter-encode.</li>
  <li>Jika sebuah direktori di-rename dengan awalan “AtoZ_”, maka direktori tersebut akan menjadi direktori ter-encode.</li>
  <li>Jika sebuah direktori di-rename dengan awalan “AtoZ_”, maka direktori tersebut akan menjadi direktori ter-encode.</li>
  <li>Setiap pembuatan direktori ter-encode (mkdir atau rename) akan tercatat ke sebuah log. Format : <b>/home/[USER]/Downloads/[Nama Direktori] → /home/[USER]/Downloads/AtoZ_[Nama Direktori]</b></li>
  <li>Metode encode pada suatu direktori juga berlaku terhadap direktori yang ada di dalamnya.(rekursif)</li>
</ol>

# Nomor 2
Selain itu Sei mengusulkan untuk membuat metode enkripsi tambahan agar data pada komputer mereka semakin aman. Berikut rancangan metode enkripsi tambahan yang dirancang oleh Sei
<ol type="a">
  <li>Jika sebuah direktori dibuat dengan awalan “RX_[Nama]”, maka direktori tersebut akan menjadi direktori terencode beserta isinya dengan perubahan nama isi sesuai kasus nomor 1 dengan algoritma tambahan ROT13 (Atbash + ROT13).</li>
  <li>Jika sebuah direktori di-rename dengan awalan “RX_[Nama]”, maka direktori tersebut akan menjadi direktori terencode beserta isinya dengan perubahan nama isi sesuai dengan kasus nomor 1 dengan algoritma tambahan Vigenere Cipher dengan key “SISOP” (Case-sensitive, Atbash + Vigenere).</li>
  <li>Apabila direktori yang terencode di-rename (Dihilangkan “RX_” nya), maka folder menjadi tidak terencode dan isi direktori tersebut akan terdecode berdasar nama aslinya.</li>
  <li>Setiap pembuatan direktori terencode (mkdir atau rename) akan tercatat ke sebuah log file beserta methodnya (apakah itu mkdir atau rename).</li>
  <li>Pada metode enkripsi ini, file-file pada direktori asli akan menjadi terpecah menjadi file-file kecil sebesar 1024 bytes, sementara jika diakses melalui filesystem rancangan Sin dan Sei akan menjadi normal. Sebagai contoh, Suatu_File.txt berukuran 3 kiloBytes pada directory asli akan menjadi 3 file kecil yakni:<br>
    Suatu_File.txt.0000<br>
Suatu_File.txt.0001<br>
Suatu_File.txt.0002<br>
    Ketika diakses melalui filesystem hanya akan muncul Suatu_File.txt
  </li>
</ol>

---

Directory yang akan menjadi sumber adalah /home/[USER]/Downloads. Jadi sebelum program dimulai, atur terlebih dahulu lokasi awalnya. Mengatur lokasi awal pada `main()`
```C
char *username = getenv("USER");
sprintf(dirpath, "/home/%s/Downloads", username);
```

Ketika sebuah direktori dibuat, maka program sebenarnya akan menjalankan fungsi `mkdir` dan secara default fungsinya adalah:
```C
static int xmp_mkdir(const char *path, mode_t mode)
```
Karena setiap membuat direktori harus membuat log (jika encode/decode), maka cek terlebih dahulu pada fungsi `mkdir`.
```C
if (strstr(filepath, "/AtoZ_") || strstr(filepath, "/RX_"))
{
    char temp[maxSize];
    sprintf(temp, "%s%s", dirpath, path);

    printf("\nCreating directory %s in %s\n", path, dirpath);
    createLog("", temp, 2);
}
```
Parameter 2 pada `createLog` adalah tanda bahwa saat ini sedang membuat direktori.

Hal yang sama juga berlaku ketika mengubah nama suatu folder. Hanya saja perlu 2 string untuk menunjuk nama awal dan nama setelah diubah.
```c
static int xmp_rename(const char *src, const char *des)
```
Perlu diperhatikan juga apakah rename menjadi encode atau decode.
```c
if (strstr(filepath, "/AtoZ_") || strstr(filepath, "/RX_"))
{
    char oldName[maxSize];
    char newName[maxSize];

    sprintf(oldName, "%s%s", dirpath, src);
    sprintf(newName, "%s%s", dirpath, des);

    printf("Renaming %s to %s in %s\n", src, des, dirpath);
    createLog(oldName, newName, 1);
}
```
Untuk membuat folder baru atau mengubah nama, keduanya memerlukan _path_ lengkap dari file yang bersangkutan untuk kemudian dilakukan operasi.
```c
char fpath[maxSize];
strcpy(fpath, find_path(path));

int res;
res = mkdir(fpath, mode);
```
```c
char file_src[maxSize];
char file_des[maxSize];

strcpy(file_src, find_path(src));
strcpy(file_des, find_path(des));

int res;
res = rename(file_src, file_des);
```

Namun ketika mengakses suatu folder, perlu untuk mengetahui apakah folder tersebut harus encode, maka dari itu perlu untuk memeriksa nama di depan terlebih dahulu. Apakah itu 'AtoZ_' atau 'RX_'. (Isi di dalam fungsi findpath)
```c
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
```
`toMirror` berguna untuk menandai folder sekarang sesuai nomor 1 sedangkan `toROT` nomor 2. Jika perlu dilakukan encode/decode, ambil parent dari folder dengan mengambil dari path sampai sebelum nama folder.
```c
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
```

Fungsi ini berlaku secara rekursif yang mana bisa dilakukan dengan perulangan. Di dalam juga perlu diperiksa untuk file, jangan sampai mengubah ekstensi dari filenya. Oleh karena itu ketika rekursi dilakukan, periksa ekstensi sebelum encode/decode. Jika tidak ada ekstensi, maka bisa langsung melakukan encoding/decoding.
```c
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
```

Ketika file sistem membaca folder/file juga perlu adanya encode/decode agar file dapat terdeteksi dari folder sumber. Filesistem akan membaca dan masuk fungsi `readdir`.
```c
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
```
Kurang lebih isinya sama, yaitu ketika mendapatkan properti dari suatu file, ubah dengan melakukan decode. Tapi karena encode/decode pada soal sama saja, maka fungsinya cukup 1 untuk 1 algoritma encoding.
```c
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
```

Ketika mengubah/membuat folder, `createLog` akan menerima 3 parameter, yaitu nama awal, nama akhir dan mode (apakah rename atau mkdir). Jika 1, pesan yang tercetak pada log adalah (Rename), sedangkan jika 2 akan tercetak (Mkdir).
```c
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
```

Sekarang untuk enkripsi 'AtoZ', yaitu dengan membalikkan (ABCD...XYZ) menjadi (ZYXW...CBA) dan caranya dengan menjumlahkan 'A' dengan 'Z' dikurangi char saat ini ('a' dan 'z' jika bukan kapital).
```c
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
```
Untuk enkripsi 'RX' akan menambah nilai 13 (ABC.. menjadi NOP), sedangkan (NOP.. menjadi ABC). Dengan kata lain sebelum huruf 'N', karakter akan bertambah nilai ASCII sebanyak 13, sedangkan 'N' dan seterusnya harus dikurangi 13. Kemudian pada soal nomor 2 harus menggabungkan Atbash dengan ROT13, jadi bisa langsung mengenkripsi hasilnya ke kode AtoZ.
```c
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
```
Metode enkripsi terakhir adalah Vigenere. Pertama dapatkan indeks karakter (a = 0, b = 1, dst) kemudian ditambahkan dengan key (SISOP). Apabila terdapat spasi, abaikan spasi dan indeks dari key tidak bertambah. Oleh karena itu buat indeks untuk menunjukkan indeks dari key. Sama seperti sebelumnya bahwa setelah enkripsi, kode harus dienkripsi lagi dengan Atbash.
```c
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
```
