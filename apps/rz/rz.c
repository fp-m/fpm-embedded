//
// Receive files with ZMODEM/YMODEM/XMODEM protocol.
//
// Copyright (c) 2020 Ross Bamford
// Copyright (c) 2025 Serge Vakulenko
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <fpm/fs.h>
#include <alloca.h>

#include "zmodem.h"

// Spec says a data packet is max 1024 bytes, but add some headroom...
#define DATA_BUF_LEN 2048

//
// Implementation-defined receive character function.
//
ZRESULT zm_recv()
{
    // TODO: ignore ^C in FP/M somehow.
    return fpm_getchar();
}

//
// Implementation-defined send character function.
//
ZRESULT zm_send(uint8_t chr)
{
    fpm_putchar(chr);
    return OK;
}

//
// Receive file.
//
void rz()
{
    fpm_puts("rz waiting for receive.\r\n");
    fpm_delay_msec(100);

    uint8_t data_buf[DATA_BUF_LEN];
    uint16_t count;
    uint32_t received_data_size = 0;
    ZHDR hdr;
    file_t *fdest = alloca(f_sizeof_file_t());
    fs_result_t fs_status = FR_NO_FILE;
    zmodem_t z = {};

startframe:
    while (true) {
        uint16_t result = zm_await_header(&z, &hdr);

        switch (result) {
        case CANCELLED:
            fpm_delay_msec(500);
            fpm_printf("Transfer cancelled by remote; Bailing...\n");
            goto cleanup;
        case OK:
            // Got valid header.

            switch (hdr.type) {
            case ZRQINIT:
            case ZEOF:
                // Got ZRQINIT or ZEOF.

                result = zm_send_flags_hdr(ZRINIT, CANOVIO | CANFC32, 0, 0, 0);

                if (result == CANCELLED) {
                    fpm_delay_msec(500);
                    fpm_printf("Transfer cancelled by remote; Bailing...\n");
                    goto cleanup;
                } else if (result == OK) {
                    // Send ZRINIT was OK.
                } else if (result == CLOSED) {
                    fpm_delay_msec(500);
                    fpm_printf("Connection closed prematurely; Bailing...\n");
                    goto cleanup;
                }
                continue;

            case ZFIN:
                // Got ZFIN.

                result = zm_send_pos_hdr(ZFIN, 0);

                if (result == CANCELLED) {
                    fpm_delay_msec(500);
                    fpm_printf("Transfer cancelled by remote; Bailing...\n");
                    goto cleanup;
                } else if (result == OK) {
                    // Send ZFIN was OK.
                } else if (result == CLOSED) {
                    fpm_delay_msec(500);
                    fpm_printf("Connection closed prematurely; Bailing...\n");
                    goto cleanup;
                }

                fpm_delay_msec(500);
                fpm_printf("Transfer complete; Received %u byte(s)\n", received_data_size);
                goto cleanup;

            case ZFILE:
                // Got ZFILE.

                switch (hdr.flags.f0) {
                case 0: // no special treatment - default to ZCBIN
                case ZCBIN:
                    // --> Binary receive.
                    break;
                case ZCNL:
                    // --> ASCII Receive: ignored, not supported.
                    break;
                case ZCRESUM:
                    // --> Resume interrupted transfer: ignored, not supported.
                    break;
                default:
                    // WARN: Invalid conversion flag hdr.flags.f0.
                    // Ignored - assuming binary.
                }

                count = DATA_BUF_LEN;
                result = zm_read_data_block(&z, data_buf, &count);
                // Result of data block read is [0x%04x] (got %d character(s))", result, count

                if (result == CANCELLED) {
                    fpm_delay_msec(500);
                    fpm_printf("Transfer cancelled by remote; Bailing...\n");
                    goto cleanup;
                } else if (!IS_ERROR(result)) {
                    // Receiving file: name in data_buf

                    fs_status = f_open(fdest, (char*)data_buf, FA_WRITE | FA_CREATE_ALWAYS);
                    if (fs_status != FR_OK) {
                        fpm_delay_msec(500);
                        fpm_printf("%s: %s\r\n", data_buf, f_strerror(fs_status));
                        goto cleanup;
                    }

                    result = zm_send_pos_hdr(ZRPOS, received_data_size);

                    if (result == CANCELLED) {
                        fpm_delay_msec(500);
                        fpm_printf("Transfer cancelled by remote; Bailing...\n");
                        goto cleanup;
                    } else if (result == OK) {
                        // Send ZRPOS was OK.
                    } else if (result == CLOSED) {
                        fpm_delay_msec(500);
                        fpm_printf("Connection closed prematurely; Bailing...\n");
                        goto cleanup;
                    }
                }

                // TODO care about XON that will follow?
                continue;

            case ZDATA:
                // Got ZDATA.

                while (true) {
                    count = DATA_BUF_LEN;
                    result = zm_read_data_block(&z, data_buf, &count);
                    // Result of data block read is [0x%04x] (got %d character(s)), result, count

                    if (fs_status != FR_OK) {
                        fpm_delay_msec(500);
                        fpm_printf("Received data before open file; Bailing...\n");
                        goto cleanup;
                    }

                    if (result == CANCELLED) {
                        fpm_delay_msec(500);
                        fpm_printf("Transfer cancelled by remote; Bailing...\n");
                        goto cleanup;
                    } else if (!IS_ERROR(result)) {
                        // Received %d byte(s) of data, count

                        unsigned nbytes_written = 0;
                        fs_status = f_write(fdest, data_buf, count - 1, &nbytes_written);
                        if (fs_status != FR_OK) {
                            fpm_delay_msec(500);
                            fpm_printf("Write error: %s\r\n", f_strerror(fs_status));
                            goto cleanup;
                        }
                        if (nbytes_written != count - 1) {
                            // Out of disk space.
                            fpm_delay_msec(500);
                            fpm_printf("Not enough space on device\r\n");
                            goto cleanup;
                        }

                        received_data_size += (count - 1);

                        if (result == GOT_CRCE) {
                            // End of frame, header follows, no ZACK expected.
                            // Got CRCE; Frame done [NOACK] [Pos: 0x%08x], received_data_size
                            break;
                        } else if (result == GOT_CRCG) {
                            // Frame continues, non-stop (another data packet follows)
                            // Got CRCG; Frame continues [NOACK] [Pos: 0x%08x], received_data_size
                            continue;
                        } else if (result == GOT_CRCQ) {
                            // Frame continues, ZACK required
                            // Got CRCQ; Frame continues [ACK] [Pos: 0x%08x], received_data_size

                            result = zm_send_pos_hdr(ZACK, received_data_size);

                            if (result == CANCELLED) {
                                fpm_delay_msec(500);
                                fpm_printf("Transfer cancelled by remote; Bailing...\n");
                                goto cleanup;
                            } else if (result == OK) {
                                // Send ZACK was OK.
                            } else if (result == CLOSED) {
                                fpm_delay_msec(500);
                                fpm_printf("Connection closed prematurely; Bailing...\n");
                                goto cleanup;
                            }

                            continue;
                        } else if (result == GOT_CRCW) {
                            // End of frame, header follows, ZACK expected.
                            // Got CRCW; Frame done [ACK] [Pos: 0x%08x], received_data_size);

                            result = zm_send_pos_hdr(ZACK, received_data_size);

                            if (result == CANCELLED) {
                                fpm_delay_msec(500);
                                fpm_printf("Transfer cancelled by remote; Bailing...\n");
                                goto cleanup;
                            } else if (result == OK) {
                                // Send ZACK was OK.
                            } else if (result == CLOSED) {
                                fpm_delay_msec(500);
                                fpm_printf("Connection closed prematurely; Bailing...\n");
                                goto cleanup;
                            }

                            break;
                        }

                    } else {
                        // Error while receiving block: 0x%04x, result

                        result = zm_send_pos_hdr(ZRPOS, received_data_size);

                        if (result == CANCELLED) {
                            fpm_delay_msec(500);
                            fpm_printf("Transfer cancelled by remote; Bailing...\n");
                            goto cleanup;
                        } else if (result == OK) {
                            // Send ZRPOS was OK.
                            goto startframe;
                        } else if (result == CLOSED) {
                            fpm_delay_msec(500);
                            fpm_printf("Connection closed prematurely; Bailing...\n");
                            goto cleanup;
                        }
                    }
                }

                continue;

            default:
                //PRINTF("WARN: Ignoring unknown header type 0x%02x\n", hdr.type);
                continue;
            }

            break;
        case BAD_CRC:
            // Didn't get valid header - CRC Check failed.

            result = zm_send_pos_hdr(ZNAK, received_data_size);

            if (result == CANCELLED) {
                fpm_delay_msec(500);
                fpm_printf("Transfer cancelled by remote; Bailing...\n");
                goto cleanup;
            } else if (result == OK) {
                // Send ZNACK was OK.
            } else if (result == CLOSED) {
                fpm_delay_msec(500);
                fpm_printf("Connection closed prematurely; Bailing...\n");
                goto cleanup;
            }

            continue;
        default:
            // Didn't get valid header - result is 0x%04x, result

            result = zm_send_pos_hdr(ZNAK, received_data_size);

            if (result == CANCELLED) {
                fpm_delay_msec(500);
                fpm_printf("Transfer cancelled by remote; Bailing...\n");
                goto cleanup;
            } else if (result == OK) {
                // Send ZNACK was OK.
            } else if (result == CLOSED) {
                fpm_delay_msec(500);
                fpm_printf("Connection closed prematurely; Bailing...\n");
                goto cleanup;
            }
            continue;
        }
    }

cleanup:
    if (fs_status == FR_OK) {
        f_close(fdest);
    }
}

int main(int argc, char **argv)
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct fpm_opt opt = {};

    while (fpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        fpm_puts("Usage: rz\r\n");
        return 0;
    }

    // Invoked without arguments: receive file.
    rz();
}
