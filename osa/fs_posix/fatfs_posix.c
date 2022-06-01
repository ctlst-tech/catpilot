
 #include <string.h>
 #include <stdio.h>
 #include "fatfs_posix.h"
 #undef strerror_r

 FILE *__iob[MAX_FILES];

 const char *sys_errlist[] =
 {
     "OK",
     "Operation not permitted",
     "No such file or directory",
     "No such process",
     "Interrupted system call",
     "I/O error",
     "No such device or address",
     "Argument list too long",
     "Exec format error",
     "Bad file number",
     "No child processes",
     "Try again",
     "Out of memory",
     "Permission denied",
     "Bad address",
     "Block device required",
     "Device or resource busy",
     "File exists",
     "Cross-device link",
     "No such device",
     "Not a directory",
     "Is a directory",
     "Invalid argument",
     "File table overflow",
     "Too many open files",
     "Not a typewriter",
     "Text file busy",
     "File too large",
     "No space left on device",
     "Illegal seek",
     "Read-only file system",
     "Too many links",
     "Broken pipe",
     "Math argument out of domain of func",
     "Math result not representable",
     "Bad Message",
     NULL
 };

 // =============================================

 int isatty(int fileno)
 {
     if(fileno >= 0 && fileno <= 2)
         return(1);
     return 0;
 }


 int
 fgetc(FILE *stream)
 {
     int c;

     if(stream == NULL)
     {
         errno = EBADF;                            // Bad File Number
         return(EOF);
     }

     if ((stream->flags & __SRD) == 0)
         return EOF;

     if ((stream->flags & __SUNGET) != 0) {
         stream->flags &= ~__SUNGET;
         stream->len++;
         return stream->unget;
     }

     if (stream->flags & __SSTR) {
         c = *stream->buf;
         if (c == '\0') {
             stream->flags |= __SEOF;
             return EOF;
         } else {
             stream->buf++;
         }
     } else {
         if(!stream->get)
         {
             return(EOF);
         }
         // get character from device or file
         c = stream->get(stream);
         if (c < 0) {
             /* if != _FDEV_ERR, assume its _FDEV_EOF */
             stream->flags |= (c == _FDEV_ERR)? __SERR: __SEOF;
             return EOF;
         }
     }

     stream->len++;
     return (c);
 }

 int getc(FILE *fp) {
     return (fgetc (fp));
 }

 int
 fputc(int c, FILE *stream)
 {
     errno = 0;
     int ret;

     if(stream != stdout && stream != stderr)
     {
         return(fatfs_putc(c,stream));
     }

     // TTY outputs

     if ((stream->flags & __SWR) == 0)
         return EOF;

     if (stream->flags & __SSTR) {
         if (stream->len < stream->size)
             *stream->buf++ = c;
         stream->len++;
         return c;
     } else {
         if(!stream->put)
         {
             return(EOF);
         }
         ret = stream->put(c, stream);
         if(ret != EOF)
             stream->len++;
         return(ret);
     }
 }

 void clearerr(FILE *stream)
 {
     stream->flags = 0;
 }


 #ifndef IO_MACROS

 int
 getchar()
 {
     return(fgetc(stdin));
 }


 int
 putchar(int c)
 {
     return(fputc(c,stdout));
 }
 #endif

 /*
 int
 ungetc(int c, FILE *stream)
 {
     int fd = fileno(stream);
     if(!isatty(fd))
         return(EOF);

     if(c == EOF)
         return EOF;
     if((stream->flags & __SUNGET) != 0 )
         return EOF;
     if ((stream->flags & __SRD) == 0 )
         return EOF;

     stream->flags |= __SUNGET;
     stream->flags &= ~__SEOF;

     stream->unget = c;
     stream->len--;

     return (c);
 }
 */
 #ifndef IO_MACROS
 // =============================================

 int
 putc(int c, FILE *stream)
 {
     return(fputc(c, stream));
 }

 #endif

 // =============================================

 char *
 fgets(char *str, int size, FILE *stream)
 {
     int c;
     int ind = 0;
     while(size--)
     {
         c = fgetc(stream);
         if(c == EOF)
         {
             if( ind == 0)
                 return(NULL);
             break;
         }
         if(c == '\n')
             break;
         if(c == 0x08)
         {
              if(ind > 0)
                 --ind;
             continue;
         }
         str[ind++] = c;
     }
     str[ind] = 0;
     return(str);
 }


 char *
 gets (char *p)
 {
     char *s;
     int n;

     s = fgets (p, MAXLN, stdin);
     if (s == 0)
         return (0);
     n = strlen (p);
     if (n && p[n - 1] == '\n')
         p[n - 1] = 0;
     return (s);
 }

 int
 fputs(const char *str, FILE *stream)
 {
     while(*str)
     {
         if(fputc(*str, stream) == EOF)
             return(EOF);
         ++str;
     }
     return(0);
 }


 #ifndef IO_MACROS

 int
 puts(const char *str)
 {
     while(*str)
     {
         if(fputc(*str, stdout) == EOF)
             return(EOF);
         ++str;
     }
     return ( fputc('\n',stdout) );
 }

 #endif


 // =============================================
 // =============================================
 // =============================================
 // =============================================

 int feof(FILE *stream)
 {
     if(stream->flags & __SEOF)
         return(1);
     return(0);
 }


 int fgetpos(FILE *stream, size_t *pos)
 {
     long offset = ftell(stream);
     *pos = offset;
     if(offset == -1)
         return(-1);
     return( 0 );
 }


 int fseek(FILE *stream, long offset, int whence)
 {
     long ret;

     int fn = fileno(stream);
     if(fn < 0)
         return(-1);

     ret  = lseek(fn, offset, whence);

     if(ret == -1)
         return(-1);

     return(0);
 }


 int fsetpos(FILE *stream, size_t *pos)
 {
     return (fseek(stream, (size_t) *pos, SEEK_SET) );
 }


 long ftell(FILE *stream)
 {
     errno = 0;

     int fn = fileno(stream);
     if(isatty(fn))
         return(-1);
     // fileno_to_fatfs checks for fd out of bounds
     FIL *fh = fileno_to_fatfs(fn);
     if ( fh == NULL )
     {
         errno = EBADF;
         return(-1);
     }

     return( fh->fptr );
 }


 off_t lseek(int fileno, off_t position, int whence)
 {
     FRESULT res;
     FIL *fh;
     errno = 0;
     FILE *stream;


     // fileno_to_fatfs checks for fd out of bounds
     fh = fileno_to_fatfs(fileno);
     if(fh == NULL)
     {
         errno = EMFILE;
         return(-1);
     }
     if(isatty(fileno))
         return(-1);


     stream = fileno_to_stream(fileno);
     stream->flags |= __SUNGET;

     if(whence == SEEK_END)
         position += f_size(fh);
     else if(whence==SEEK_CUR)
         position += fh->fptr;

     res = f_lseek(fh, position);
     if(res)
     {
         errno = fatfs_to_errno(res);
         return -1;
     }
     return (fh->fptr);
 }


 void rewind( FILE *stream)
 {
     fseek(stream, 0L, SEEK_SET);
 }

 // =============================================
 // =============================================
 // =============================================
 // =============================================

 int fatfs_close(int fileno)
 {
     FILE *stream;
     FIL *fh;
     int res;

     errno = 0;

     // checks if fileno out of bounds
     stream = fileno_to_stream(fileno);
     if(stream == NULL)
     {
         return(-1);
     }

     // fileno_to_fatfs checks for fileno out of bounds
     fh = fileno_to_fatfs(fileno);
     if(fh == NULL)
     {
         return(-1);
     }
     res = f_close(fh);
     free_file_descriptor(fileno);
     if (res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
     return(0);
 }


 int fileno(FILE *stream)
 {
     int fileno;

     if(stream == NULL)
     {
         errno = EBADF;
         return(-1);
     }

     for(fileno=0; fileno<MAX_FILES; ++fileno)
     {
         if ( __iob[fileno] == stream)
             return(fileno);
     }
     return(-1);
 }


 FILE *fileno_to_stream(int fileno)
 {
     FILE *stream;
     if(fileno < 0 || fileno >= MAX_FILES)
     {
         errno = EBADF;
         return(NULL);
     }

     stream = __iob[fileno];
     if(stream == NULL)
     {
         errno = EBADF;
         return(NULL);
     }
     return(stream);
 }


 FILE *fopen(const char *path, const char *mode)
 {
     int flags = posix_fopen_modes_to_open(mode);
     int fileno = fatfs_open(path, flags);

     // checks if fileno out of bounds
     return( fileno_to_stream(fileno) );
 }


 size_t __wrap_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
 {
     size_t count = size * nmemb;
     int fn = fileno(stream);
     ssize_t ret;

     // read() checks for fn out of bounds
     ret = fatfs_read(fn, ptr, count);
     if(ret < 0)
         return(0);

     return((size_t) ret);
 }


 int ftruncate(int fd, off_t length)
 {
     errno = 0;
     FIL *fh;
     FRESULT rc;

     if(isatty(fd))
         return(-1);
     // fileno_to_fatfs checks for fd out of bounds
     fh = fileno_to_fatfs(fd);
     if(fh == NULL)
     {
         return(-1);
     }
     rc = f_lseek(fh, length);
     if (rc != FR_OK)
     {
         errno = fatfs_to_errno(rc);
         return(-1);
     }
     rc = f_truncate(fh);
     if (rc != FR_OK)
     {
         errno = fatfs_to_errno(rc);
         return(-1);
     }
     return(0);
 }


 size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
 {
     size_t count = size * nmemb;
     int fn = fileno(stream);
     ssize_t ret;

     // write () checks for fn out of bounds
     ret =  fatfs_write(fn, ptr, count);

     if(ret < 0)
         return(0);

     return((size_t) ret);
 }




 int fatfs_open(const char *pathname, int flags)
 {
     int fileno;
     int fatfs_modes;
     FILE *stream;
     FIL *fh;
     int res;

     errno = 0;
 // FIXME Assume here that the disk interface mmc_init was already called
 #if 0
 // Checks Disk status
     res = mmc_init(0);

     if(res != RES_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
 #endif

     if((flags & O_ACCMODE) == O_RDWR)
         fatfs_modes = FA_READ | FA_WRITE;
     else if((flags & O_ACCMODE) == O_RDONLY)
         fatfs_modes = FA_READ;
     else
         fatfs_modes = FA_WRITE;

     if(flags & O_CREAT)
     {
         if(flags & O_TRUNC)
             fatfs_modes |= FA_CREATE_ALWAYS;
         else
             fatfs_modes |= FA_OPEN_ALWAYS;
     }

     fileno = new_file_descriptor();

     // checks if fileno out of bounds
     stream = fileno_to_stream(fileno);
     if(stream == NULL)
     {
         free_file_descriptor(fileno);
         return(-1);
     }

     // fileno_to_fatfs checks for fileno out of bounds
     fh = fileno_to_fatfs(fileno);
     if(fh == NULL)
     {
         free_file_descriptor(fileno);
         errno = EBADF;
         return(-1);
     }
     res = f_open(fh, pathname, (BYTE) (fatfs_modes & 0xff));
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         free_file_descriptor(fileno);
         return(-1);
     }
     if(flags & O_APPEND)
     {
         res = f_lseek(fh, f_size(fh));
         if (res != FR_OK)
         {
             errno = fatfs_to_errno(res);
             f_close(fh);
             free_file_descriptor(fileno);
             return(-1);
         }
     }

     if((flags & O_ACCMODE) == O_RDWR)
     {
         // FIXME fdevopen should do this
         stream->put = fatfs_putc;
         stream->get = fatfs_getc;
         stream->flags = _FDEV_SETUP_RW;
     }
     else if((flags & O_ACCMODE) == O_RDONLY)
     {
         // FIXME fdevopen should do this
         stream->put = NULL;
         stream->get = fatfs_getc;
         stream->flags = _FDEV_SETUP_READ;
     }
     else
     {
         // FIXME fdevopen should do this
         stream->put = fatfs_putc;
         stream->get = NULL;
         stream->flags = _FDEV_SETUP_WRITE;
     }

     return(fileno);
 }


 ssize_t fatfs_read(int fd, void *buf, size_t count)
 {
     UINT size;
     UINT bytes = count;
     int res;
     int ret;
     FIL *fh;
     FILE *stream;

     //FIXME
     *(char *) buf = 0;

     errno = 0;

     // TTY read function
     // FIXME should we really be blocking ???
     stream = fileno_to_stream(fd);
     if(stream == stdin)
     {
         char *ptr = (char *) buf;
         // ungetc is undefined for read
         stream->flags |= __SUNGET;
         size = 0;
         while(count--)
         {
             ret = fgetc(stream);
             if(ret < 0)
                 break;

             *ptr++ = ret;
             ++size;
         }
         return(size);
     }
     if(stream == stdout || stream == stderr)
     {
         return(-1);
     }

     // fileno_to_fatfs checks for fd out of bounds
     fh = fileno_to_fatfs(fd);
     if ( fh == NULL )
     {
         errno = EBADF;
         return(-1);
     }

     res = f_read(fh, (void *) buf, bytes, &size);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
     return ((ssize_t) size);
 }



 void sync(void)
 {
     FIL *fh;
     int i;

     for(i=0;i<MAX_FILES;++i)
     {
         if(isatty(i))
             continue;

         // fileno_to_fatfs checks for i out of bounds
         fh = fileno_to_fatfs(i);
         if(fh == NULL)
             continue;

         (void ) syncfs(i);
     }
 }


 int syncfs(int fd)
 {
     FIL *fh;
     FRESULT res;
     FILE *stream;

     errno = 0;

     if(isatty(fd))
     {
         errno = EBADF;
         return(-1);
     }
     stream = fileno_to_stream(fd);
     // reset unget on sync
     stream->flags |= __SUNGET;

     // fileno_to_fatfs checks for fd out of bounds
     fh = fileno_to_fatfs(fd);
     if(fh == NULL)
     {
         errno = EBADF;
         return(-1);
     }

     res  = f_sync ( fh );
     if (res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
     return(0);
 }




 int truncate(const char *path, off_t length)
 {
     errno = 0;
     FIL fh;
     FRESULT rc;

     rc = f_open(&fh , path, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
     if (rc != FR_OK)
     {
         errno = fatfs_to_errno(rc);
         return(-1);
     }
     rc = f_lseek(&fh, length);
     if (rc != FR_OK)
     {
         errno = fatfs_to_errno(rc);
         return(-1);
     }
     rc = f_truncate(&fh);
     if (rc != FR_OK)
     {
         errno = fatfs_to_errno(rc);
         return(-1);
     }
     return(0);
 }



 ssize_t fatfs_write(int fd, const void *buf, size_t count)
 {
     UINT size;
     UINT bytes = count;
     FRESULT res;
     FIL *fh;
     FILE *stream;
     errno = 0;

     // TTY read function
     stream = fileno_to_stream(fd);
     if(stream == stdout || stream == stderr)
     {
         char *ptr = (char *) buf;
         size = 0;
         while(count--)
         {
             int c,ret;
             c = *ptr++;
             ret = fputc(c, stream);
             if(c != ret)
                 break;

             ++size;
         }
         return(size);
     }
     if(stream == stdin)
     {
         return(-1);
     }

     // fileno_to_fatfs checks for fd out of bounds
     fh = fileno_to_fatfs(fd);
     if ( fh == NULL )
     {
         errno = EBADF;
         return(-1);
     }

     res = f_write(fh, buf, bytes, &size);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
     return ((ssize_t) size);
 }

 FILE * __wrap_freopen ( const char * filename, const char * mode, FILE * stream )
 {
     int fn = fileno(stream);
     int ret = fatfs_close(fn);
     if (ret < 0) {
         return NULL;
     }
     return fopen(filename, mode);
 }


 int __wrap_fclose(FILE *stream)
 {
     int fn = fileno(stream);
     if(fn < 0)
         return(EOF);

     return( fatfs_close(fn) );
 }
 // =============================================
 // =============================================
 // =============================================
 // =============================================

 /*
 void dump_stat(struct stat *sp)
 {
     mode_t mode = sp->st_mode;

     printf("\tSize:  %lu\n", (uint32_t)sp->st_size);

     printf("\tType:  ");
     if(S_ISDIR(mode))
         printf("DIR\n");
     else if(S_ISREG(mode))
         printf("File\n");
     else
         printf("Unknown\n");


     printf("\tMode:  %lo\n", (uint32_t)sp->st_mode);
     printf("\tUID:   %lu\n", (uint32_t)sp->st_uid);
     printf("\tGID:   %lu\n", (uint32_t)sp->st_gid);
     printf("\tatime: %s\n",mctime((time_t)sp->st_atime));
     printf("\tmtime: %s\n",mctime((time_t)sp->st_mtime));
     printf("\tctime: %s\n",mctime((time_t)sp->st_ctime));
 }
 */
 #if 0

 int fstat(int fd, struct stat *buf)
 {
     FIL *fh;
     FRESULT rc;

     if(isatty(fd))
         return(-1);

     //FIXME TODO
     return(-1);

 }
 #endif


 int stat(const char *name, struct stat *buf)
 {
     FILINFO info;
     int res;
     time_t epoch;
     uint16_t mode;
     errno = 0;

     // f_stat does not handle / or . as root directory
     if (strcmp(name,"/") == 0 || strcmp(name,".") == 0)
     {
         buf->st_atime = 0;
         buf->st_mtime = 0;
         buf->st_ctime = 0;
         buf->st_uid= 0;
         buf->st_gid= 0;
         buf->st_size = 0;
         buf->st_mode = S_IFDIR;
         return(0);
     }

     res = f_stat(name, &info);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }

     buf->st_size = info.fsize;
     epoch = fat_time_to_unix(info.fdate, info.ftime);
     buf->st_atime = epoch;                        // Access time
     buf->st_mtime = epoch;                        // Modification time
     buf->st_ctime = epoch;                        // Creation time

     // We only handle read only case
     mode = (FATFS_R | FATFS_X);
     if( !(info.fattrib & AM_RDO))
         mode |= (FATFS_W);                        // enable write if NOT read only

     if(info.fattrib & AM_SYS)
     {
         buf->st_uid= 0;
         buf->st_gid= 0;
     }
     {
         buf->st_uid=1000;
         buf->st_gid=1000;
     }

     if(info.fattrib & AM_DIR)
         mode |= S_IFDIR;
     else
         mode |= S_IFREG;
     buf->st_mode = mode;

     return(0);
 }


 int utime(const char *filename, const struct utimbuf *times)
 {

     FILINFO fno;
     uint16_t fdate,ftime;
     time_t ut = 0;
     int res;

     if(times)
         ut = times->modtime;


     unix_time_to_fat(ut, (uint16_t *) &fdate, (uint16_t *) &ftime);


     fno.fdate = fdate;
     fno.ftime = ftime;

     res = f_utime(filename, (FILINFO *) &fno);

     return( fatfs_to_errno(res) );
 }

 int64_t fs_getfree() {
     FATFS *fs;
     DWORD fre_clust, fre_sect;


     /* Get volume information and free clusters of drive 1 */
     FRESULT res = f_getfree("/", &fre_clust, &fs);
     if (res) return(res);

     /* Get total sectors and free sectors */
     fre_sect = fre_clust * fs->csize;
     return (int64_t)(fre_sect)*512;
 }


 int64_t fs_gettotal() {
     FATFS *fs;
     DWORD fre_clust, tot_sect;


     /* Get volume information and free clusters of drive 1 */
     FRESULT res = f_getfree("/", &fre_clust, &fs);
     if (res) return(res);

     /* Get total sectors and free sectors */
     tot_sect = (fs->n_fatent - 2) * fs->csize;
     return (int64_t)(tot_sect)*512;
 }
 // =============================================
 // =============================================
 // =============================================
 // =============================================

 char *basename(const char *str)
 {
     const char *base = str;
     if(!str)
         return("");
     while(*str)
     {
         if(*str++ == '/')
             base = str;
     }

     return (char*)base;
 }


 char *baseext(char *str)
 {
     char *ext = "";

     while(*str)
     {
         if(*str++ == '.')
             ext = str;
     }
     return(ext);
 }



 int chdir(const char *pathname)
 {
     errno = 0;

     int res = f_chdir(pathname);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
     return(0);
 }


 int chmod(const char *pathname, mode_t mode)
 {
     int rc;
     errno = 0;

     // FIXME for now we combine user,group and other perms and ask if anyone has write perms ?

     // Read only ???
     if ( !( mode & ( S_IWUSR | S_IWGRP | S_IWOTH)))
     {
         // file is read only
         rc = f_chmod(pathname, AM_RDO, AM_RDO);
         if (rc != FR_OK)
         {
             errno = fatfs_to_errno(rc);
             return(-1);
         }
     }

     return(0);
 }


 char *dirname(char *str)
 {
     int end = 0;
     int ind = 0;

     if(!str)
         return(0);

     while(*str)
     {
         if(*str == '/')
             end = ind;
         ++str;
         ++ind;
     }
     return &str[end];
 }

 #if 0

 int fchmod(int fd, mode_t mode)
 {
     //FIXME TODO
     return (-1);
 }
 #endif


 char *getcwd(char *pathname, int len)
 {
     int res;
     errno = 0;

     res = f_getcwd(pathname, len);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(NULL);
     }
     return(pathname);
 }


 int mkdir(const char *pathname, mode_t mode)
 {
     errno = 0;

     int res = f_mkdir(pathname);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }

     if (mode) {
         chmod(pathname, mode);
     }

     return(0);
 }


 int rename(const char *oldpath, const char *newpath)
 {
 /* Rename an object */
     int rc;
     errno = 0;
     rc = f_rename(oldpath, newpath);
     if(rc)
     {
         errno = fatfs_to_errno(rc);
         return(-1);
     }
     return(0);
 }


 int rmdir(const char *pathname)
 {
     errno = 0;
     int res = f_unlink(pathname);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
     return(0);
 }



 int unlink(const char *pathname)
 {
     errno = 0;
     int res = f_unlink(pathname);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
     return(0);
 }


 int remove(const char *pathname)
 {
     errno = 0;
     int res = f_unlink(pathname);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
     return(0);
 }

 // =============================================
 // =============================================
 // =============================================
 // =============================================
 int closedir(DIR *dirp)
 {
     int res = f_closedir (dirp);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
     return(0);
 }

 static DIR _dp;
 DIR *opendir(const char *pathdir)
 {
     int res = f_opendir((DIR *) &_dp, pathdir);
     if(res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(NULL);
     }
     return ((DIR *) &_dp);
 }

 static  dirent_t _de;
 dirent_t * readdir(DIR *dirp)
 {
     FILINFO fno;
     int len;
     int res;

     _de.d_name[0] = 0;
     res = f_readdir ( dirp, &fno );
     if(res != FR_OK || fno.fname[0] == 0)
     {
         errno = fatfs_to_errno(res);
         return(NULL);
     }
     len = strlen(fno.fname);
     strncpy(_de.d_name,fno.fname,len);
     _de.d_name[len] = 0;
     return( (dirent_t *) &_de);
 }

 // =============================================
 // =============================================
 // =============================================
 // =============================================


 void clrerror(FILE *stream)
 {
     stream->flags &= ~__SEOF;
     stream->flags &= ~__SERR;
 }


 int ferror(FILE *stream)
 {
     if(stream->flags & __SERR)
         return(1);
     return(0);
 }


 void perror(const char *s)
 {
     const char *ptr = NULL;


     if(errno >=0 && errno < EBADMSG)
         ptr = sys_errlist[errno];
     else
         ptr = sys_errlist[EBADMSG];

     if(s && *s)
         printf("%s: %s\n", s, ptr);
     else
         printf("%s\n", ptr);
 }


 char *strerror(int errnum)
 {
     return( (char *)sys_errlist[errnum] );
 }



 char *__wrap_strerror_r(int errnum, char *buf, size_t buflen)
 {
         strncpy(buf, sys_errlist[errnum], buflen);
         return(buf);
 }


 // =============================================
 // =============================================
 // =============================================
 // =============================================

 FILE *
 fdevopen(int (*put)(char, FILE *), int (*get)(FILE *))
 {
     FILE *s;

     if (put == 0 && get == 0)
         return 0;

     if ((s = calloc(1, sizeof(FILE))) == 0)
         return 0;

     s->flags = __SMALLOC;

     if (get != 0) {
         s->get = get;
         s->flags |= __SRD;
         // We assign the first device with a read discriptor to stdin
         // Only assign once
         if (stdin == 0)
             stdin = s;
     }

     if (put != 0) {
         s->put = put;
         s->flags |= __SWR;
         // NOTE: We assign the first device with a write to both STDOUT andd STDERR

         // Only assign in unassigned
         if (stdout == 0)
             stdout = s;
         if (stderr == 0)
             stderr = s;
     }

     return s;
 }


 // =============================================
 // =============================================
 // =============================================
 // =============================================

 /*
 int mkfs(char *name)
 {
     FATFS fs;
     uint8_t *mem;
     int res;
     int len;
     int c;
     char dev[4];

     len = MATCH(name,"/dev/sd");
     if(!len)
     {
         printf("Expected /dev/sda .. /dev/sdj\n");
         return(0);
     }
     // Convert /dev/sd[a-j] to 0: .. 9:
     dev[1] = ':';
     dev[2] = 0;
     c = tolower( name[len-1] );
     if(c >= 'a' && c <= ('a' + 9))
         dev[0] = (c - 'a');
     dev[3] = 0;

     // Register work area to the logical drive 0:
     res = f_mount(&fs, dev, 0);
     if(!res)
     {
         put_rc(res);
         return(0);
     }

     // Allocate memory for mkfs function
     mem = malloc(1024);
     if(!mem)
         return(0);

     // Create FAT volume on the logical drive 0
     // 2nd argument is ignored.
     res = f_mkfs(dev, FM_FAT32, 0, mem, 1024);
     if(res)
     {
         put_rc(res);
         free(mem);
         return(0);
     }
     free(mem);
     return(1);
 }
 */

 int  fatfs_getc(FILE *stream)
 {
     FIL *fh;
     UINT size;
     int res;
     uint8_t c;
     long pos;

     errno = 0;

     if(stream == NULL)
     {
         errno = EBADF;                            // Bad File Number
         return(EOF);
     }

     fh = (FIL *) fdev_get_udata(stream);
     if(fh == NULL)
     {
         errno = EBADF;                            // Bad File Number
         return(EOF);
     }

     res = f_read(fh, &c, 1, (UINT *) &size);
     if( res != FR_OK || size != 1)
     {
         errno = fatfs_to_errno(res);
         stream->flags |= __SEOF;
         return(EOF);
     }

     // AUTOMATIC end of line METHOD detection
     // ALWAYS return '\n' for ALL methods
     // History: End of line (EOL) characters sometimes differ, mostly legacy systems, and modern UNIX (which uses just '\n')
     //    '\r' ONLY
     //    '\r\n'
     //    '\n'
     // The difference was mostly from the way old mechanical printers were controlled.
     //    '\n' (New Line = NL) advanced the line
     //    '\r' (Charage Return = CR) moved the print head to start of line
     //    '\t' (Tabstop = TAB)
     //    '\f' (Form feed = FF)
     // The problem with mechanical devices is that each had differing control and time delays to deal with.
     //  (TAB, CR, NL and FF) did different things and took differing times depending on the device.
     //
     // Long before DOS UNIX took the position that controlling physical devices must be a device drivers problem only.
     // They reasoned if users had to worry about all the ugly controll and timing issues no code would be portable.
     // Therefore they made NL just a SYMBOL for the driver to determine what to do.
     // This design philosophy argued if you needed better control its better to use a real designed purposed tool for it.
     // (ie. like curses or termcap).

     // Here to deal with those other old ugly stupid pointless EOL methods we convert to just a symbol '\n'
     // FROM '\n' OR '\r'char OR '\r\n' TO '\n'
     // Note: char != '\n'
     if(c == '\r')
     {
         // PEEK forward 1 character
         pos = f_tell(fh);
         // Check for trailing '\n' or EOF
         res = f_read(fh, &c, 1, (UINT *) &size);
         if(res != FR_OK || size != 1)
         {
             // '\r' with EOF impiles '\n'
             return('\n');
         }
         // This file must be '\r' ONLY for end of line
         if(c != '\n')
         {
             // Not '\n' or EOF o move file pointer back to just after the '\r'
             f_lseek(fh, pos);
             return('\n');
         }
         c = '\n';
     }
     return(c & 0xff);
 }


 int fatfs_putc(char c, FILE *stream)
 {
     int res;
     FIL *fh;
     UINT size;

     errno = 0;
     if(stream == NULL)
     {
         errno = EBADF;                            // Bad File Number
         return(EOF);
     }

     fh = (FIL *) fdev_get_udata(stream);
     if(fh == NULL)
     {
         errno = EBADF;                            // Bad File Number
         return(EOF);
     }

     res = f_write(fh, &c, 1, (UINT *)  &size);
     if( res != FR_OK || size != 1)
     {
         errno = fatfs_to_errno(res);
         stream->flags |= __SEOF;
         return(EOF);
     }
     return(c);
 }


 int fatfs_to_errno( FRESULT Result )
 {
     switch( Result )
     {
         case FR_OK:              /* FatFS (0) Succeeded */
             return (0);          /* POSIX OK */
         case FR_DISK_ERR:        /* FatFS (1) A hard error occurred in the low level disk I/O layer */
             return (EIO);        /* POSIX Input/output error (POSIX.1) */

         case FR_INT_ERR:         /* FatFS (2) Assertion failed */
             return (EPERM);      /* POSIX Operation not permitted (POSIX.1) */

         case FR_NOT_READY:       /* FatFS (3) The physical drive cannot work */
             return (EBUSY);      /* POSIX Device or resource busy (POSIX.1) */

         case FR_NO_FILE:         /* FatFS (4) Could not find the file */
             return (ENOENT);     /* POSIX No such file or directory (POSIX.1) */

         case FR_NO_PATH:         /* FatFS (5) Could not find the path */
             return (ENOENT);     /* POSIX No such file or directory (POSIX.1) */

         case FR_INVALID_NAME:    /* FatFS (6) The path name format is invalid */
             return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

         case FR_DENIED:          /* FatFS (7) Access denied due to prohibited access or directory full */
             return (EACCES);     /* POSIX Permission denied (POSIX.1) */

         case FR_EXIST:           /* file exists */
             return (EEXIST);     /* file exists */

         case FR_INVALID_OBJECT:  /* FatFS (9) The file/directory object is invalid */
             return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

         case FR_WRITE_PROTECTED: /* FatFS (10) The physical drive is write protected */
             return(EROFS);       /* POSIX Read-only filesystem (POSIX.1) */

         case FR_INVALID_DRIVE:   /* FatFS (11) The logical drive number is invalid */
             return(ENXIO);       /* POSIX No such device or address (POSIX.1) */

         case FR_NOT_ENABLED:     /* FatFS (12) The volume has no work area */
             return (ENOSPC);     /* POSIX No space left on device (POSIX.1) */

         case FR_NO_FILESYSTEM:   /* FatFS (13) There is no valid FAT volume */
             return(ENXIO);       /* POSIX No such device or address (POSIX.1) */

         case FR_MKFS_ABORTED:    /* FatFS (14) The f_mkfs() aborted due to any parameter error */
             return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

         case FR_TIMEOUT:         /* FatFS (15) Could not get a grant to access the volume within defined period */
             return (EBUSY);      /* POSIX Device or resource busy (POSIX.1) */

         case FR_LOCKED:          /* FatFS (16) The operation is rejected according to the file sharing policy */
             return (EBUSY);      /* POSIX Device or resource busy (POSIX.1) */


         case FR_NOT_ENOUGH_CORE: /* FatFS (17) LFN working buffer could not be allocated */
             return (ENOMEM);     /* POSIX Not enough space (POSIX.1) */

         case FR_TOO_MANY_OPEN_FILES:/* FatFS (18) Number of open files > _FS_SHARE */
             return (EMFILE);     /* POSIX Too many open files (POSIX.1) */

         case FR_INVALID_PARAMETER:/* FatFS (19) Given parameter is invalid */
             return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

     }
     return (EBADMSG);            /* POSIX Bad message (POSIX.1) */
 }



 int fatfs_to_fileno(FIL *fh)
 {
     int i;

     FILE *stream;

     if(fh == NULL)
     {
         errno = EBADF;
         return(-1);
     }

     for(i=0;i<MAX_FILES;++i)
     {
         stream = __iob[i];
         if(stream)
         {
             if( fh == (FIL *) fdev_get_udata(stream) )
                 return(i);
         }
     }
     errno = EBADF;
     return(-1);
 }

 /*
   mktime replacement from Samba
  */
 static time_t replace_mktime(const struct tm *t)
 {
     time_t  epoch = 0;
     int n;
     int mon [] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }, y, m, i;
     const unsigned MINUTE = 60;
     const unsigned HOUR = 60*MINUTE;
     const unsigned DAY = 24*HOUR;
     const unsigned YEAR = 365*DAY;

     if (t->tm_year < 70) {
         return((time_t)-1);
     }

     n = t->tm_year + 1900 - 1;
     epoch = (t->tm_year - 70) * YEAR +
         ((n / 4 - n / 100 + n / 400) - (1969 / 4 - 1969 / 100 + 1969 / 400)) * DAY;

     y = t->tm_year + 1900;
     m = 0;

     for(i = 0; i < t->tm_mon; i++) {
         epoch += mon [m] * DAY;
         if(m == 1 && y % 4 == 0 && (y % 100 != 0 || y % 400 == 0))
             epoch += DAY;

         if(++m > 11) {
             m = 0;
             y++;
         }
     }

     epoch += (t->tm_mday - 1) * DAY;
     epoch += t->tm_hour * HOUR + t->tm_min * MINUTE + t->tm_sec;

     return epoch;
 }


 time_t fat_time_to_unix(uint16_t date, uint16_t time)
 {
     struct tm tp;
     time_t unix;

     memset(&tp, 0, sizeof(struct tm));

     tp.tm_sec = (time << 1) & 0x3e;               // 2 second resolution
     tp.tm_min = ((time >> 5) & 0x3f);
     tp.tm_hour = ((time >> 11) & 0x1f);
     tp.tm_mday = (date & 0x1f);
     tp.tm_mon = ((date >> 5) & 0x0f) - 1;
     tp.tm_year = ((date >> 9) & 0x7f) + 80;
     unix = replace_mktime( &tp );
     return( unix );
 }


 void unix_time_to_fat(time_t epoch, uint16_t *date, uint16_t *time)
 {
     // TODO: add gmtime implementation
//      struct tm *t = gmtime((time_t *) &epoch);

//  /* Pack date and time into a uint32_t variable */
//      *date = ((uint16_t)(t->tm_year - 80) << 9)
//          | (((uint16_t)t->tm_mon+1) << 5)
//          | (((uint16_t)t->tm_mday));

//      *time = ((uint16_t)t->tm_hour << 11)
//          | ((uint16_t)t->tm_min << 5)
//          | ((uint16_t)t->tm_sec >> 1);
 }


 FIL *fileno_to_fatfs(int fileno)
 {
     FILE *stream;
     FIL *fh;

     if(isatty( fileno ))
     {
         errno = EBADF;
         return(NULL);
     }

     // checks if fileno out of bounds
     stream = fileno_to_stream(fileno);
     if( stream == NULL )
         return(NULL);

     fh = fdev_get_udata(stream);
     if(fh == NULL)
     {
         errno = EBADF;
         return(NULL);
     }
     return(fh);
 }




 int free_file_descriptor(int fileno)
 {
     FILE *stream;
     FIL *fh;

     if(isatty( fileno ))
     {
         errno = EBADF;
         return(-1);
     }

     // checks if fileno out of bounds
     stream = fileno_to_stream(fileno);
     if(stream == NULL)
     {
         return(-1);
     }

     fh = fdev_get_udata(stream);

     if(fh != NULL)
     {
         free(fh);
     }

     if(stream->buf != NULL && stream->flags & __SMALLOC)
     {
         free(stream->buf);
     }

     __iob[fileno]  = NULL;
     free(stream);
     return(fileno);
 }



 // =============================================

 int new_file_descriptor( void )
 {
     int i;
     FILE *stream;
     FIL *fh;

     for(i=0;i<MAX_FILES;++i)
     {
         if(isatty(i))
             continue;
         if( __iob[i] == NULL)
         {
             stream = (FILE *) calloc(sizeof(FILE),1);
             if(stream == NULL)
             {
                 errno = ENOMEM;
                 return(-1);
             }
             fh = (FIL *) calloc(sizeof(FIL),1);
             if(fh == NULL)
             {
                 free(stream);
                 errno = ENOMEM;
                 return(-1);
             }

             __iob[i]  = stream;
             fdev_set_udata(stream, (void *) fh);
             return(i);
         }
     }
     errno = ENFILE;
     return(-1);
 }


 int posix_fopen_modes_to_open(const char *mode)
 {
     int flag = 0;

     if(modecmp(mode,"r") || modecmp(mode,"rb"))
     {
         flag = O_RDONLY;
         return(flag);
     }
     if(modecmp(mode,"r+") || modecmp(mode, "r+b" ) || modecmp(mode, "rb+" ))
     {
         flag = O_RDWR | O_TRUNC;
         return(flag);
     }
     if(modecmp(mode,"w") || modecmp(mode,"wb"))
     {
         flag = O_WRONLY | O_CREAT | O_TRUNC;
         return(flag);
     }
     if(modecmp(mode,"w+") || modecmp(mode, "w+b" ) || modecmp(mode, "wb+" ))
     {
         flag = O_RDWR | O_CREAT | O_TRUNC;
         return(flag);
     }
     if(modecmp(mode,"a") || modecmp(mode,"ab"))
     {
         flag = O_WRONLY | O_CREAT | O_APPEND;
         return(flag);
     }
     if(modecmp(mode,"a+") || modecmp(mode, "a+b" ) || modecmp(mode, "ab+" ))
     {
         flag = O_RDWR | O_CREAT | O_APPEND;
         return(-1);
     }
     return(-1);                                   // nvalid mode
 }

 // =============================================
 // =============================================
 // =============================================
 // =============================================



 /*
 static void _fprintf_putc(struct _printf_t *p, char ch)
 {
         p->sent++;
         fputc(ch, (FILE *) p->buffer);
 }

 */

 int
 __wrap_fprintf(FILE *fp, const char *fmt, ...)
 {
     va_list va;
     char* buf;
     int16_t len, i;
     va_start(va, fmt);
     len = vasprintf(&buf, fmt, va);
     if (len > 0) {
         for(i = 0; i < len; i++) {
             fputc(buf[i], fp);
         }
     } else {
         return -1;
     }
     va_end(va);

     free(buf);

     return len;
 }

 /*
   fsync file
  */
 int
 fsync(int fileno)
 {
     FILE *stream;
     FIL *fh;
     int res;

     errno = 0;

     // checks if fileno out of bounds
     stream = fileno_to_stream(fileno);
     if(stream == NULL)
     {
         return(-1);
     }

     // fileno_to_fatfs checks for fileno out of bounds
     fh = fileno_to_fatfs(fileno);
     if(fh == NULL)
     {
         return(-1);
     }
     res = f_sync(fh);
     if (res != FR_OK)
     {
         errno = fatfs_to_errno(res);
         return(-1);
     }
     return(0);
 }