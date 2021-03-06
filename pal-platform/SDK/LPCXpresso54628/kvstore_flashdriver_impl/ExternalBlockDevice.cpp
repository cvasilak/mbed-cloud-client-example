/* mbed Microcontroller Library
 * Copyright (c) 2017-2019 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "ExternalBlockDevice.h"

#if DEVICE_FLASH

#include "flash_api.h"

#if 0
#define debug_print(...) printf(__VA_ARGS__)
#else
#define debug_print(...)
#endif

namespace mbed {

BlockDevice* BlockDevice::get_default_instance()
{
    static ExternalBlockDevice bd;
    return &bd;
}

static inline bool is_aligned(uint32_t number, uint32_t alignment)
{
    if ((number % alignment) != 0) {
        return false;
    } else {
        return true;
    }
}

ExternalBlockDevice::ExternalBlockDevice()
{
}

ExternalBlockDevice::~ExternalBlockDevice()
{
}

int ExternalBlockDevice::init()
{
    int ret = 0;

    if (flash_init(NULL) != 0) {
        ret = -1;
    }
    debug_print("ExternalBlockDevice::init() <= %d\r\n", ret);
    return ret;
}

int ExternalBlockDevice::deinit()
{
    int ret = 0;

    if (flash_free(NULL) != 0) {
        ret = -1;
    }
    debug_print("ExternalBlockDevice::deinit() <= %d\r\n", ret);
    return ret;
}

int ExternalBlockDevice::read(void *buffer, bd_addr_t addr, bd_size_t size)
{
    debug_print("ExternalBlockDevice::read() => %llu %llu\r\n", addr, size);
    int ret = flash_read(NULL, flash_get_start_address(NULL)+addr, (uint8_t *) buffer, size);
    debug_print("ExternalBlockDevice::read() <= %d\r\n", ret);
    return ret;
}

int ExternalBlockDevice::program(const void *buffer, bd_addr_t addr, bd_size_t size)
{
    debug_print("ExternalBlockDevice::program() => %llu %llu\r\n", addr, size);
    uint32_t page_size = get_program_size();
    uint32_t flash_size = flash_get_size(NULL);
    uint32_t flash_start_address = flash_get_start_address(NULL);
    uint32_t address = flash_start_address + addr;

    // address should be aligned to page size
    if (!is_aligned(address, page_size) || (!buffer) ||
            ((address + size) > (flash_start_address + flash_size))) {
        debug_print("ExternalBlockDevice::program(1) <= -1\r\n");
        return -1;
    }

    int ret = 0;
    uint32_t end_address = address + size;
    const uint8_t* source = (const uint8_t*) buffer;

    while ((address < end_address) && (ret == 0)) {

        ret = flash_program_page(NULL, address, source, page_size);

        address += page_size;
        source += page_size;
    }
    debug_print("ExternalBlockDevice::program(2) <= %d\r\n", ret);

    return ret;
}

int ExternalBlockDevice::erase(bd_addr_t address, bd_size_t size)
{
    debug_print("ExternalBlockDevice::erase() => %llu %llu\r\n", address, size);

    int ret = 0;
    uint32_t offset = flash_get_start_address(NULL);

    uint32_t erase_address = address + offset;
    uint32_t end_address = address + offset + size;

    /* erase one sector at a time */
    while ((erase_address < end_address) && (ret == 0)) {

        /* erase current sector */
        ret = flash_erase_sector(NULL, erase_address);

        /* set next erase address */
        erase_address += flash_get_sector_size(NULL, erase_address);
    }

    debug_print("ExternalBlockDevice::erase() <= %d\r\n", ret);
    return ret;
}

bd_size_t ExternalBlockDevice::get_read_size() const
{
    return 1;
}

bd_size_t ExternalBlockDevice::get_program_size() const
{
    return flash_get_page_size(NULL);
}

bd_size_t ExternalBlockDevice::get_erase_size() const
{
    return flash_get_sector_size(NULL, flash_get_start_address(NULL));
}

bd_size_t ExternalBlockDevice::size() const
{
    return flash_get_size(NULL);
}

int ExternalBlockDevice::get_erase_value() const
{
    return flash_get_erase_value(NULL);
}

const char* ExternalBlockDevice::get_type() const
{
    return "STORAGE";
}

}

#endif
