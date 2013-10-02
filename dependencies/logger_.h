#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

int LOGGER(FILE *file, const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    int size = vsprintf(buffer, format, args);
    if (fwrite(buffer, 1, size, file) != size) {
        return -1;
    }
    va_end(args);
    return 0;
}

int SENDR_SEND_LOGGER(FILE *file, int seq, int lfs0, int lfs1, int lar) {
    if (seq <= lfs1) {
        return LOGGER(file, "Resend Seq#: %i, LAR: %i, LFS: [%i to %i], time:%i\n", seq, lar, lfs0, lfs1, time(NULL));
    } else {
        return LOGGER(file, "Send Seq#: %i, LAR: %i, LFS: [%i to %i], time:%i\n", seq, lar, lfs0, lfs1, time(NULL));
    }
}

int SENDR_RECV_LOGGER(FILE *file, int seq, int lfs0, int lfs1, int lar) {
    return LOGGER(file, "Receive ACK#: %i, LAR: %i, LFS: [%i to %i], time:%i\n", seq, lar, lfs0, lfs1, time(NULL));
}

int RECVR_SEND_LOGGER(FILE *file, int seq, int lfRead, int lfRecv, int laf, int resend) {
    if (resend) {
        return LOGGER(file, "Resend Seq#: %i, lfRead: %i, lfRcvd: %i, LAF: %i, time:%i\n", seq, lfRead, lfRecv, laf, time(NULL));
    } else {
        return LOGGER(file, "Send Seq#: %i, lfRead: %i, lfRcvd: %i, LAF: %i, time:%i\n", seq, lfRead, lfRecv, laf, time(NULL));
    }
}

int RECVR_RECV_LOGGER(FILE *file, int seq, int lfRead, int lfRecv, int laf) {
    return LOGGER(file, "Receive Seq#: %i, lfRead: %i, lfRcvd: %i, LAF: %i, time:%i\n", seq, lfRead, lfRecv, laf, time(NULL));
}
