#include <errno.h>
#include <sys/stat.h>

int _close(int file) {
    (void)file;
    return -1;
}
int _fstat(int file, struct stat *st) {
    (void)file;

    st->st_mode = S_IFCHR;
    return 0;
}
int _isatty(int file) {
    (void)file;
    return 1;
}
int _lseek(int file, int ptr, int dir) {
    (void)ptr;
    (void)dir;
    (void)file;
    return 0;
}
int _read(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

// Если нужен printf через UART/ITM, добавьте вывод символов сюда:
int _write(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    // HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
