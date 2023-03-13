#include <rpm/api.h>
#include <rpm/fs.h>

const char *f_strerror(fs_result_t errnum)
{
    switch (errnum) {
    case FR_OK:                  return "Succeeded";
    case FR_DISK_ERR:            return "Input/output error";
    case FR_INT_ERR:             return "Assertion failed";
    case FR_NOT_READY:           return "The physical drive cannot work";
    case FR_NO_FILE:             return "No such file or directory";
    case FR_NO_PATH:             return "No such path name";
    case FR_INVALID_NAME:        return "The path name format is invalid";
    case FR_DENIED:              return "Permission denied";
    case FR_EXIST:               return "Access denied due to prohibited access";
    case FR_INVALID_OBJECT:      return "The file/directory object is invalid";
    case FR_WRITE_PROTECTED:     return "The physical drive is write protected";
    case FR_INVALID_DRIVE:       return "The logical drive number is invalid";
    case FR_NOT_ENABLED:         return "The volume has no work area";
    case FR_NO_FILESYSTEM:       return "There is no valid FAT volume";
    case FR_MKFS_ABORTED:        return "Cannot create filesystem: disk too small/big";
    case FR_TIMEOUT:             return "Operation timed out";
    case FR_LOCKED:              return "Resource busy";
    case FR_NOT_ENOUGH_CORE:     return "Cannot allocate memory";
    case FR_TOO_MANY_OPEN_FILES: return "Too many open files";
    case FR_INVALID_PARAMETER:   return "Given parameter is invalid";
    default: break;
    }

    static char buf[40];
    rpm_snprintf(buf, sizeof(buf), "Unknown error: %u", errnum);
    return buf;
}
