#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <stdlib.h>
#include <string_view>

#include <iostream>

using namespace std;

namespace img_lib {

const static int align = 4;
const static int num_color = 3;

const static uint16_t BMP_SIGN = 19778; // 'BM == 19778 = M{0100 1101} B{0100 0010}'
const static uint32_t def_offset = 54; // размер двух заголовков
const static uint32_t def_size_info_header = 40; // размер второго заголовка
const static int32_t def_H_DPI = 11811; // 300 DPI
const static int32_t def_V_DPI = 11811; // 300 DPI
const static uint16_t def_color_bits = 24;
const static int32_t def_num_relevant_color = 0x1000000; // 16777216 colors


PACKED_STRUCT_BEGIN BitmapFileHeader {
    // поля заголовка Bitmap File Header
    uint16_t sign = BMP_SIGN; // 2
    uint32_t size = 0; // 4 // сумма размера двух заголовков и размера данных (отступ * высоту)
    uint32_t reserved = 0; // 4
    uint32_t offset = def_offset;// 4
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    // поля заголовка Bitmap Info Header
    uint32_t size_header = def_size_info_header; // 4
    int32_t width = 0; // 4
    int32_t height = 0; // 4
    uint16_t num_plane = 1; // 2
    uint16_t color_bits = def_color_bits; // 2
    uint32_t type_compress = 0; // 4
    uint32_t data_size = 0; // 4 // отступ * высоту
    int32_t h_resolution = def_H_DPI; // 4
    int32_t v_resolution = def_V_DPI; // 4
    int32_t num_color = 0; // 4
    int32_t num_relevant_color = def_num_relevant_color; // 4
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * num_color + num_color) / align);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    const int w = image.GetWidth();
    const int h = image.GetHeight();
    const int stride = GetBMPStride(w);
    BitmapFileHeader BFHeader;
    BFHeader.size = def_offset + static_cast<uint32_t>(stride * h);
    BitmapInfoHeader BIHeader;
    BIHeader.width = static_cast<int32_t>(w);
    BIHeader.height = static_cast<int32_t>(h);
    BIHeader.data_size = static_cast<uint32_t>(stride * h);
    out.write(reinterpret_cast<char*>(&BFHeader), sizeof(BFHeader));
    out.write(reinterpret_cast<char*>(&BIHeader), sizeof(BIHeader));
    std::vector<char> buff(stride);
    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), stride);
    }
    return out.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    // открываем поток с флагом ios::binary
    ifstream ifs(file, ios::binary);
    BitmapFileHeader BFHeader;
    // читаем первые 14 байт из файла и пишем их в структуру
    ifs.read(reinterpret_cast<char*>(&BFHeader), sizeof(BFHeader));
    // проверка подписи и количества считанных байт
    if (ifs.gcount() != sizeof(BFHeader) || BFHeader.sign != BMP_SIGN) {
        return {};
    }
    BitmapInfoHeader BIHeader;
    // читаем следующие 40 байт и пишем их в заголовок
    ifs.read(reinterpret_cast<char*>(&BIHeader), sizeof(BIHeader));
    // проверка считанных байт
    if (ifs.gcount() != sizeof(BIHeader)) {
        return {};
    }
    const int w = BIHeader.width;
    const int h = BIHeader.height;
    const int stride = GetBMPStride(w);
    Image result(w, h, stride, Color::Black());
    std::vector<char> buff(stride);

    for (int y = h - 1; y >= 0; --y) {
        if (ifs.eof() || ifs.bad() || ifs.fail()) {
            return {};
        }
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), stride);
        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib
