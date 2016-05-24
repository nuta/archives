#include <string.h>
#include <resea.h>
#include <resea/fs.h>
#include <resea/channel.h>
#include <resea/storage_device.h>

extern uint8_t fat_test_disk_img[];

static void storage_device_mock_handler(channel_t ch, payload_t *m) {

    offset_t offset;
    size_t size;
    void *data;

    switch (EXTRACT_MSGID(m)) {
    case MSGID(storage_device, read):
        offset = EXTRACT(m, storage_device, read, offset);
        size   = EXTRACT(m, storage_device, read, size);
        data   = &fat_test_disk_img[offset];
        send_storage_device_read_reply(ch, OK, data, size);
        break;
    default:
        BUG("storage_mock: unsupported msg");
    }
}


void fat_test(void) {
    channel_t mock_ch, fat_ch;
    result_t r;

    // launch a mock
    mock_ch = create_channel();
    call_channel_register(connect_to_local(1), mock_ch,
        INTERFACE(storage_device), &r);
    set_channel_handler(mock_ch, storage_device_mock_handler);

    // connect to the fat
    fat_ch = create_channel();
    call_channel_connect(connect_to_local(1), fat_ch,
        INTERFACE(fs), &r);

    // open file
    ident_t file;
    char const *path = "docs/hello.txt";
    call_fs_open(fat_ch, (uchar_t *) path, strlen(path), FS_FILEMODE_READ,
        &r, &file);

    TEST_EXPECT_DESC("[docs/hello.txt] fs.open return OK", r == OK);

    char *data;
    size_t size;
    call_fs_read(fat_ch, file, 0, 5,
        &r, (void **) &data, &size);

    TEST_EXPECT_DESC("[docs/hello.txt] fs.read returns OK", r == OK);
    TEST_EXPECT_DESC("[docs/hello.txt] fs.read returns correct size", size == 5);
    TEST_EXPECT_DESC("[docs/hello.txt] fs.read returns data", !strncmp(data, "hello", 5));

    TEST_END();
}
